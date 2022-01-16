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

using namespace std;
using namespace PowerFake;
using namespace PowerFake::internal;

Functions ReadFunctionsList(vector<string> wrapper_files, bool verbose)
{
    const string_view::size_type BUF_SIZE = 1 * 1024 * 1024;
    const string_view::size_type MIN_ACCEPTABLE_INCOMPLETE_POS = BUF_SIZE - 10 * 1024;
    const string_view PREFIX { PFK_PROTO_PREFIX };
    const string_view START_MARKER { PFK_PROTO_START };
    const string_view END_MARKER { PFK_PROTO_END };
    Functions functions;
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
                    auto infostr = strbuf.substr(p_start, p_end - p_start + END_MARKER.size());
                    if (verbose)
                        cout << "Found Prototype: " << infostr << endl;
                    if (auto finfo = GetFunctionInfo(infostr))
                        functions.push_back(finfo.value());
                    else
                        cout << "Skipped: " << infostr << endl;
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
    auto nstart = s.rfind("::");
    if (nstart == string::npos)
        finfo.prototype.name = s;
    else
        finfo.prototype.name = s.substr(nstart);

    is >> s >> finfo.prototype.alias >> s;
    std::getline(is, s, '|');
    string_view sv = s;
    auto ret_end = sv.find('(');
    auto param_end = sv.rfind(')');
    auto param_start = sv.rfind('(', param_end);
    finfo.prototype.return_type = sv.substr(1, ret_end - 2);
    finfo.prototype.params = sv.substr(param_start, param_end - param_start + 1);
    if (sv.substr(ret_end, 3) != "(*)") // a class member
        finfo.prototype.name = string(sv.substr(ret_end + 1, sv.rfind("::", param_start) - ret_end - 1)) + finfo.prototype.name;

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
        finfo.prototype.name = s + finfo.prototype.name;
    return finfo;
}
