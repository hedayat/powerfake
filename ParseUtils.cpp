/*
 * ParseUtils.cpp
 *
 *  Created on: ۱۵ ژانویهٔ ۲۰۲۲
 *      Author: Hedayat Vatankhah <hedayat.fwd@gmail.com>
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
    const string_view PREFIX { PFK_PROTO_PREFIX };
    const string_view START_MARKER { PFK_PROTO_START };
    const string_view END_MARKER { PFK_PROTO_END };
    Functions functions;
    set<string> found_prototypes;
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
            auto p_start = strbuf.find(START_MARKER);
            while (p_start != string_view::npos)
            {
                auto p_end = strbuf.find(PREFIX, p_start + START_MARKER.size());
                while (p_end != string_view::npos &&
                        strbuf.substr(p_end, END_MARKER.size()) != END_MARKER)
                {
                    if (strbuf.substr(p_end, START_MARKER.size()) == START_MARKER)
                        p_start = p_end;
                    p_end = strbuf.find(PREFIX, p_end + PREFIX.size());
                }

                if (p_end != string_view::npos)
                {
                    auto infostr = strbuf.substr(p_start,
                        p_end - p_start + END_MARKER.size());
                    /*
                     * There can be duplicate entries in the binary, so we
                     * use an std::set to eliminate duplicates
                     */
                    found_prototypes.insert(string(infostr));
                }
                else
                    break;
                p_start = strbuf.find(START_MARKER, p_end + END_MARKER.size());
            }

            if (wfile)
            {
                string_view::size_type keep_start;
                if (p_start == string_view::npos || p_start < MIN_ACCEPTABLE_INCOMPLETE_POS)
                    keep_start = strbuf.size() - START_MARKER.size();
                else
                    keep_start = p_start;
                auto handover_data = strbuf.substr(keep_start);
                memcpy(buff_mem, handover_data.data(), handover_data.size());
                read_start = handover_data.size();
            }
        } while (wfile);
    }

    for (const auto &infostr: found_prototypes)
    {
        if (verbose)
            cout << "Found Prototype: " << infostr << endl;
        if (auto finfo = GetFunctionInfo(infostr))
            functions.push_back(finfo.value());
        else
            cout << "Skipped: " << infostr << endl;
    }
    return functions;
}

std::optional<PowerFake::internal::FunctionInfo> GetFunctionInfo(string_view function_str)
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
    string_view sv = s;
    auto ret_end = sv.find('(');
    auto param_end = sv.rfind(')');
    auto param_start = sv.find('(', ret_end + 1);
    finfo.prototype.return_type = FixConstPlacement(sv.substr(1, ret_end - 2));
    finfo.prototype.params = FixParamsConstPlacement(
        sv.substr(param_start, param_end - param_start + 1));
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
    auto name_end = demangled.find_first_of("<[(");
    if (name_end == string::npos)
        return demangled;

    while (demangled[name_end] == '<')
    {
        auto checkop = demangled.find_first_of(" [(", name_end);
        if (checkop - name_end <= 2) // <( | <<( | << < => should be operator definition
        {
            name_end = demangled.find_first_of("<[(", checkop);
            continue;
        }
        unsigned tpl = 1;
        for (++name_end; tpl; ++name_end)
        {
            if (name_end >= demangled.size())
                throw runtime_error(
                    "(BUG) Error parsing function name for: " + string(demangled));
            if (demangled[name_end] == '>')
                --tpl;
            else if (demangled[name_end] == '<')
                ++tpl;
        }
        name_end = demangled.find_first_of("<[(", name_end);
    }

    end_pos = name_end;
    auto name_begin = demangled.find_last_of(" >:", name_end);
    if (name_begin == string::npos)
        return demangled.substr(0, name_end);
    if (demangled[name_begin] == '>')
    {
        int tpl = 1;
        for (name_begin--;
                name_begin > 0 && (tpl || (demangled[name_begin] != ' '
                        && demangled[name_begin] != ':')); --name_begin)
        {
            if (demangled[name_begin] == '>')
                tpl++;
            else if (demangled[name_begin] == '<')
                tpl--;
        }
    }
    if (name_begin > 0)
    {
        auto op_begin = demangled.find_last_of(" :", name_begin-1);
        std::string op;
        if (op_begin == string::npos)
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

std::string FixConstPlacement(std::string_view compile_type)
{
    std::string suffix;
    auto param_end = compile_type.find_last_of(")>");
    if (param_end != string_view::npos && compile_type[param_end] == ')') // function type
    {
        auto fparan_start = compile_type.find('(');
        auto param_start = compile_type.find('(', fparan_start + 1);
        if (compile_type[fparan_start - 1] != ' ')
            suffix = ' ';
        suffix += compile_type.substr(fparan_start, param_start - fparan_start);
        suffix += FixParamsConstPlacement(
            compile_type.substr(param_start, param_end - param_start + 1));
        suffix += compile_type.substr(param_end + 1);
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

    auto lookup = compile_type.find_first_of("*&<", start_pos);
    if (lookup == string_view::npos)
    {
        res = compile_type.substr(start_pos);
        if (add_const) // actually we don't expect this to happen! const without */&?!
            res += (res.back() == ' ' ? "" : " ") + "const"s;
        return res + suffix;
    }

    switch (compile_type[lookup])
    {
        case '&':
        case '*':
            if (add_const)
            {
                res = compile_type.substr(start_pos, lookup - start_pos);
                res += (res.back() == ' ' ? "" : " ") + "const"s
                        + FixSpaces(compile_type.substr(lookup));
            }
            else
                res = compile_type;
            break;
        case '<':
        {
            auto tplend = compile_type.rfind('>');
            res = compile_type.substr(start_pos, ++lookup - start_pos);
            res += FixParamsConstPlacement(
                compile_type.substr(lookup, tplend - lookup));
            res += '>';
            if (add_const)
                res += (res.back() == ' ' ? "" : " ") + "const"s;
            res += FixSpaces(compile_type.substr(tplend + 1));
            break;
        }
    }
    return res + suffix;
}

std::string FixParamsConstPlacement(std::string_view params)
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
        res += FixConstPlacement(p);
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
