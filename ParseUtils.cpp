/*
 * ParseUtils.cpp
 *
 *  Created on: ۱۵ ژانویهٔ ۲۰۲۲
 *
 *  Copyright Hedayat Vatankhah <hedayat.fwd@gmail.com>, 2022.
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *     (See accompanying file LICENSE_1_0.txt or copy at
 *           http://www.boost.org/LICENSE_1_0.txt)
 */

#include "ParseUtils.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <set>

using namespace std;
using namespace PowerFake;
using namespace PowerFake::internal;


ExtendedPrototype::ExtendedPrototype(std::string ret, std::string name,
    std::string params, uint32_t qual) :
        FunctionPrototype(ret, name, params, qual), expanded_params(
            SplitParams(this->params))
{
}

Functions ReadFunctionsList(vector<string> wrapper_files, bool verbose)
{
    const string_view::size_type BUF_SIZE = 1 * 1024 * 1024;
    const string_view::size_type MIN_ACCEPTABLE_INCOMPLETE_POS = BUF_SIZE - 10 * 1024;
    const string_view PREFIX { PFK_TAG_PREFIX };
    const string_view PT_START_MARKER { PFK_PROTO_START };
    const string_view PT_END_MARKER { PFK_PROTO_END };
    const string_view TH_START_MARKER { PFK_TYPEHINT_START };
    const string_view TH_END_MARKER { PFK_TYPEHINT_END };
    Functions functions;
    set<string> found_prototypes;
    set<string> found_typehints;
    char buff_mem[BUF_SIZE];
    for (const auto &wf_name: wrapper_files)
    {
        if (verbose)
            cout << "\nLooking for marked functions in " << wf_name
                << "\n------------------------------------------------------------------------------------------------------"
                << endl;
        ifstream wfile(wf_name, ios::binary);
        int read_start = 0;
        do
        {
            wfile.read(buff_mem + read_start, BUF_SIZE - read_start);
            if (wfile.gcount() <= 0) continue;

            string_view strbuf(buff_mem, read_start + wfile.gcount());
            auto [p_start, found] = FindStrings(PREFIX, { PT_START_MARKER,
                TH_START_MARKER }, strbuf);
            while (p_start != string_view::npos)
            {
                string_view start_marker = found ? TH_START_MARKER : PT_START_MARKER;
                string_view end_marker = found ? TH_END_MARKER : PT_END_MARKER;
                auto [p_end, marker_found] = FindStrings(PREFIX, { end_marker,
                        PT_START_MARKER, TH_START_MARKER }, strbuf,
                    p_start + start_marker.size());
                while (p_end != string_view::npos && marker_found)
                {
                    p_start = p_end;
                    found = marker_found - 1;
                    start_marker = found ? TH_START_MARKER : PT_START_MARKER;
                    end_marker = found ? TH_END_MARKER : PT_END_MARKER;

                    tie(p_end, marker_found) = FindStrings(PREFIX, { end_marker,
                            PT_START_MARKER, TH_START_MARKER }, strbuf,
                        p_start + start_marker.size());
                }

                if (p_end != string_view::npos)
                {
                    auto infostr = strbuf.substr(p_start,
                        p_end - p_start + end_marker.size());
                    /*
                     * There can be duplicate entries in the binary, so we
                     * use an std::set to eliminate duplicates
                     */
                    if (found)
                        found_typehints.insert(string(infostr));
                    else
                        found_prototypes.insert(string(infostr));
                }
                else
                    break;
                std::tie(p_start, found) = FindStrings(PREFIX, { PT_START_MARKER,
                    TH_START_MARKER }, strbuf, p_end + end_marker.size());
            }

            if (wfile)
            {
                string_view::size_type keep_start;
                if (p_start == string_view::npos || p_start < MIN_ACCEPTABLE_INCOMPLETE_POS)
                    keep_start = strbuf.size() - PT_START_MARKER.size();
                else
                    keep_start = p_start;
                auto handover_data = strbuf.substr(keep_start);
                memcpy(buff_mem, handover_data.data(), handover_data.size());
                read_start = handover_data.size();
            }
        } while (wfile);
    }

    map<string, string> type_hints;
    for (const auto &infostr: found_typehints)
    {
        if (verbose)
            cout << "Found Type Hint: " << infostr << endl;
        if (auto tinfo = GetTypeHint(infostr))
        {
            auto [it, inserted] = type_hints.insert(tinfo.value());
            if (!inserted)
            {
                cerr << "Duplicate type hint for type: " << tinfo.value().first
                        << " => " << tinfo.value().second;
            }
        }
        else
            cout << "Skipped: " << infostr << endl;
    }

    for (const auto &infostr: found_prototypes)
    {
        if (verbose)
            cout << "Found Prototype: " << infostr << endl;
        if (auto finfo = GetFunctionInfo(infostr, type_hints))
            functions.push_back(finfo.value());
        else
            cout << "Skipped: " << infostr << endl;
    }
    return functions;
}

std::optional<PowerFake::internal::FunctionInfo> GetFunctionInfo(
    string_view function_str, const TypeHintMap &type_hints)
{
    /*
     * Found Prototype: PFKPrototypeStart: WRAPPED | SampleClass::CallThis | Folan__pfkalias__43 | void (FakeTest::SampleClass::*)() const | PFKPrototypeEnd
     * Found Prototype: PFKPrototypeStart: WRAPPED | SampleClass::StaticFunc | Folan__pfkalias__40 | void (*)() | FakeTest::SampleClass | PFKPrototypeEnd
     * Found Prototype: PFKPrototypeStart: HIDDEN | called_by_normal_func | Folan__pfkalias__37 | void (*)(int) | PFKPrototypeEnd
     */
    FunctionInfo finfo;

    istringstream is { string(function_str) };
    string s;
    is >> s >> s;
    if (s == "WRAPPED")
        finfo.fake_type = FakeType::WRAPPED;
    else if (s == "HIDDEN")
        finfo.fake_type = FakeType::HIDDEN;
    else
        return {};

    is >> s >> s;
    finfo.prototype.name = s;
    auto nstart = finfo.prototype.name.rfind("::");
    if (nstart == string::npos)
        nstart = 0;

    is >> s >> finfo.prototype.alias >> s;
    std::getline(is, s, '|');
    if (!is) return {};
    string_view sv = s;
    auto ret_end = sv.find('(');
    auto param_end = sv.rfind(')');
    auto param_start = sv.find('(', ret_end + 1);
    finfo.prototype.return_type = NormalizeType(sv.substr(1, ret_end - 2),
        type_hints);
    finfo.prototype.params = NormalizeParameters(
        sv.substr(param_start, param_end - param_start + 1), type_hints);
    if (sv.substr(ret_end, 3) != "(*)") // a class member
        finfo.prototype.name = string(
            sv.substr(ret_end + 1, sv.rfind("::", param_start) - ret_end - 1))
                + finfo.prototype.name.substr(nstart);

    istringstream qualis { s.substr(param_end + 1) };
    while (qualis >> s)
    {
        if (s == "const")
            finfo.prototype.qual |= Qualifiers::CONST;
        else if (s == "volatile")
            finfo.prototype.qual |= Qualifiers::VOLATILE;
        else if (s == "noexcept")
            finfo.prototype.qual |= Qualifiers::NOEXCEPT;
        else if (s == "&")
            finfo.prototype.qual |= Qualifiers::LV_REF;
        else if (s == "&&")
            finfo.prototype.qual |= Qualifiers::RV_REF;
        else if (s == "const&")
            finfo.prototype.qual |= Qualifiers::CONST_REF; // if exists ;)
        else
            cerr << "Unknown qualifier: " << s << endl;
    }

    is >> s;
    if (s != PFK_PROTO_END)
        finfo.prototype.name = s + finfo.prototype.name.substr(nstart);
    return finfo;
}

std::optional<std::pair<std::string, std::string>> GetTypeHint(
    std::string_view typehint_str)
{
    /*
     * PFKTypeHintStart: std::string | std::basic_string<char, allocator<char> > | PFKTypeHintEnd
     */
    string ct_type, demangled_type, s;

    istringstream is { string(typehint_str) };
    is >> s;

    is.ignore(10, ' ');
    std::getline(is, ct_type, '|');
    if (!is) return {};
    auto ltrim = ct_type.find_last_not_of(' ');
    ct_type.resize(ltrim + 1);

    is.ignore(10, ' ');
    std::getline(is, demangled_type, '|');
    if (!is) return {};
    ltrim = demangled_type.find_last_not_of(' ');
    demangled_type.resize(ltrim + 1);
    return pair{ ct_type, demangled_type };
}

ExtendedPrototype ParseDemangledFunction(std::string_view demangled,
    unsigned name_start, unsigned name_end)
{
    std::string ret, name, params;
    uint32_t qual = Qualifiers::NO_QUAL;
    if (name_start > 0)
    {
        unsigned tpl = 0;
        for (--name_start;
                name_start > 0 && (tpl || demangled[name_start] != ' ');
                --name_start)
        {
            if (demangled[name_start] == '>')
                tpl++;
            else if (demangled[name_start] == '<')
                tpl--;
        }
        ret = demangled.substr(0, name_start);
    }
    if (name_start)
        ++name_start;
    name = demangled.substr(name_start, name_end - name_start);
    auto pstart = demangled.find('(', name_end);
    auto pend = demangled.rfind(')');
    ++pend;
    params = demangled.substr(pstart, pend - pstart);

    istringstream is { string(demangled.substr(pend)) };
    string w;
    while (is >> w)
        qual |= QualifierFromStr(w);

    return { ret, name, params, qual };
}

std::string_view FunctionName(std::string_view demangled, unsigned &start_pos,
    unsigned &end_pos)
{
    start_pos = 0;
    end_pos = demangled.size();
    auto name_end = demangled.rfind(")");
    if (name_end == string_view::npos)
        return demangled;

    int num_pr = 1;
    for (--name_end; name_end > 0; --name_end)
    {
        if (demangled[name_end] == ')')
            ++num_pr;
        else if (demangled[name_end] == '(')
            --num_pr;
        if (num_pr == 0) break;
    }
    if (num_pr)
        throw runtime_error(
            "(BUG) Error parsing function name for: " + string(demangled));

    // look for ABI tag, but not operator[]
    if (demangled[name_end - 1] == ']' && demangled[name_end - 2] != '[')
        name_end = demangled.rfind('[', name_end - 1);
    end_pos = name_end;
    if (name_end == string_view::npos)
        throw runtime_error(
            "(BUG) Error parsing function name for: " + string(demangled));

    auto name_begin = demangled.find_last_of(" >:", name_end);
    if (name_begin == string_view::npos)
        return demangled.substr(0, name_end);

    if (demangled[name_begin] == '>')
    {
        int tpl = 1;
        for (name_begin--; name_begin > 0 && tpl; --name_begin)
        {
            if ((demangled[name_begin] == ' ' || demangled[name_begin] == ':')
                    && demangled.substr(name_begin + 1, strlen("operator>"))
                            == "operator>")
            {
                tpl = 0;
                break;
            }
            else if (demangled[name_begin] == '>')
                tpl++;
            else if (demangled[name_begin] == '<')
                tpl--;
        }
        // "operator<< <>"
        if (demangled[name_begin] == ' ' && demangled[name_begin + 1] == '<')
            --name_begin;
        name_begin = demangled.find_last_of(" :", name_begin);
    }
    if (name_begin > 0)
    {
        auto op_begin = demangled.find_last_of(" :", name_begin-1);
        std::string op;
        if (op_begin == string_view::npos)
        {
            op = demangled.substr(0, name_begin);
            op_begin = 0;
        }
        else
        {
            ++op_begin;
            op = demangled.substr(op_begin, name_begin - op_begin);
        }
        if (op == "operator")
            name_begin = op_begin;
        else
            ++name_begin;
    }
    start_pos = name_begin;
    return demangled.substr(name_begin, name_end - name_begin);
}

PowerFake::internal::Qualifiers QualifierFromStr(std::string_view qs)
{
    if (qs == "&")
        return Qualifiers::LV_REF;
    else if (qs == "&&")
        return Qualifiers::RV_REF;
    else if (qs == "const&")
        return Qualifiers::CONST_REF;
    else if (qs == "const")
        return Qualifiers::CONST;
    else if (qs == "volatile")
        return Qualifiers::VOLATILE;
    else if (qs == "noexcept")
        return Qualifiers::NOEXCEPT;
    return Qualifiers::NO_QUAL;
}

std::string NormalizeType(std::string_view compile_type,
    const TypeHintMap &type_hints)
{
    std::string suffix;
    auto param_end = compile_type.find_last_of(")>");
    if (param_end != string_view::npos && compile_type[param_end] == ')') // function type
    {
        auto fparan_start = compile_type.find('(');
        auto param_start = compile_type.find('(', fparan_start + 1);
        suffix = ' ';
        suffix += compile_type.substr(fparan_start, param_start - fparan_start);
        suffix += NormalizeParameters(
            compile_type.substr(param_start, param_end - param_start + 1),
            type_hints);
        suffix += compile_type.substr(param_end + 1);
        if (compile_type[fparan_start - 1] == ' ')
            --fparan_start;
        compile_type = compile_type.substr(0, fparan_start);
    }

    std::string res;
    unsigned start_pos = 0;
    bool add_const = false;

    if (compile_type.substr(0, strlen("const ")) == "const ")
    {
        add_const = true;
        start_pos = strlen("const ");
    }

    auto translate = [&type_hints](string_view t) {
        if (t.back() == ' ')
            t = t.substr(0, t.size() - 1);
        auto f = type_hints.find(string(t));
        if (f != type_hints.end())
            return f->second;
        return string(t);
    };
    auto lookup = compile_type.find_first_of("*&<", start_pos);
    if (lookup == string_view::npos)
    {
        res = translate(compile_type.substr(start_pos));
        if (add_const) // actually we don't expect this to happen! const without */&?!
            res += " const";
        return res + suffix;
    }

    switch (compile_type[lookup])
    {
        case '&':
        case '*':
            if (add_const)
            {
                res = translate(
                    compile_type.substr(start_pos, lookup - start_pos));
                res += " const" + FixSpaces(compile_type.substr(lookup));
            }
            else
                res = translate(compile_type);
            break;
        case '<':
        {
            auto tplend = compile_type.rfind('>');
            res = translate(compile_type.substr(start_pos, lookup++ - start_pos));
            res += '<' + NormalizeParameters(
                compile_type.substr(lookup, tplend - lookup), type_hints);
            res += '>';
            res = translate(res);
            if (add_const)
                res += " const";
            res += FixSpaces(compile_type.substr(tplend + 1));
            break;
        }
    }
    return res + suffix;
}

std::string NormalizeParameters(std::string_view params,
    const TypeHintMap &type_hints)
{
    std::string res;
    if (params[0] == '(')
        res = '(';

    bool first = true;
    for (auto p: SplitParams(params))
    {
        if (!first)
            res += ", ";
        first = false;
        res += NormalizeType(p, type_hints);
    }

    if (res[0] == '(')
        res += ')';
    return res;
}

std::vector<std::string_view> SplitParams(std::string_view params)
{
    unsigned param_start = 0;
    int nested = 0;
    vector<string_view> res;

    // Note: we assume valid input

    if (params[0] == '(' && params.back() == ')')
        params = params.substr(1, params.size() - 2);

    unsigned i;
    for (i = 0; i < params.size(); ++i)
    {
        switch (params[i])
        {
            case '(':
            case '<':
                ++nested;
                break;
            case ')':
            case '>':
                --nested;
                break;
            case ' ':
                if (param_start == i)
                    ++param_start;
                break;
            case ',':
                if (nested)
                    break;
                if (i != param_start)
                    res.push_back(params.substr(param_start, i - param_start));
                param_start = i + 1;
                break;
        }
    }
    if (param_start < i) // no ending parenthesis
        res.push_back(params.substr(param_start));

    return res;
}

std::string FixSpaces(std::string_view type_str)
{
    std::string res;
    bool word_mode = type_str[0] != '*' && type_str[0] != '&';
    unsigned pos = 0;
    do
    {
        if (word_mode)
        {
            auto next = type_str.find_first_of("*&", pos);
            if (next == string_view::npos)
                break;
            int skip = (type_str[next - 1] == ' ' ? 1 : 0);
            res += type_str.substr(pos, next - skip - pos);
            pos = next;
        }
        else
        {
            auto next = type_str.find_first_not_of("*& ", pos);
            if (next == string_view::npos)
                break;
            res += type_str.substr(pos, next - pos);
            if (type_str[next - 1] != ' ')
                res += ' ';
            pos = next;
        }
        word_mode = !word_mode;
    }
    while (true);
    res += type_str.substr(pos);
    return res;
}

std::pair<std::string_view::size_type, int> FindStrings(std::string_view prefix,
    std::vector<std::string_view> srch, std::string_view str,
    std::string_view::size_type start_pos)
{
    auto pos = str.find(prefix, start_pos);

    while (pos != string_view::npos)
    {
        for (unsigned i = 0; i < srch.size(); ++i)
        {
            if (str.substr(pos, srch[i].size()) == srch[i])
                return { pos, i };
        }
        pos = str.find(prefix, pos + prefix.size());
    }

    return { pos, -1 };
}
