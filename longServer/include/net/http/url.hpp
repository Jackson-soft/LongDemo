#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/split.hpp>
#include <map>
#include <string>
#include <string_view>
#include <vector>

// [scheme:][//[userinfo@]host][/]path[?query][#fragment]

namespace Uranus
{
namespace Http
{
class URL
{
public:
    URL()  = default;
    ~URL() = default;

public:
    auto Parse(const std::string_view rawurl) -> bool
    {
        if (rawurl.empty())
            return false;
        if (rawurl == "*") {
            Path = "*";
            return true;
        }

        std::vector<std::string> result;
        boost::split(result, rawurl, boost::is_any_of("#"), boost::token_compress_on);

        boost::to_lower(Scheme);
        return false;
    }
    bool Query() { return false; }

    auto IsAbs() -> bool { return !Scheme.empty(); }

private:
    std::string Scheme;
    std::string Opaque;  // encoded opaque data
    std::string Host;    // host or host:port
    std::string Path;
    std::string RawQuery;
    std::string Fragment;
};
}  // namespace Http
}  // namespace Uranus
