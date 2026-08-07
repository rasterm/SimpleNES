#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>

#include "Controller.h"
#include "Log.h"

namespace sn
{
// trim from start (construct new string)
inline std::string ltrim(const std::string& str)
{
    std::string s(str);
    s.erase(s.begin(), std::find_if_not<decltype(s.begin()), int(int)>(s.begin(), s.end(), std::isspace));
    return s;
}

// trim from end (construct new string)
inline std::string rtrim(const std::string& str)
{
    std::string s(str);
    s.erase(std::find_if_not<decltype(s.rbegin()), int(int)>(s.rbegin(), s.rend(), std::isspace).base(), s.end());
    return s;
}

void parseControllerConf(std::string filepath, std::vector<Key>& p1, std::vector<Key>& p2)
{
    const std::string buttonStrings[] = { "A", "B", "Select", "Start", "Up", "Down", "Left", "Right" };

    std::ifstream file(filepath);
    std::string   line;
    enum
    {
        Player1,
        Player2,
        None
    } state              = None;
    unsigned int line_no = 0;
    while (std::getline(file, line))
    {
        line = rtrim(ltrim(line));
        if (line.empty() || line[0] == '#')
            continue;
        else if (line == "[Player1]")
        {
            state = Player1;
        }
        else if (line == "[Player2]")
        {
            state = Player2;
        }
        else if (state == Player1 || state == Player2)
        {
            const auto divider = line.find("=");
            const auto it = std::find(std::begin(buttonStrings), std::end(buttonStrings),
                                      ltrim(rtrim(line.substr(0, divider))));
            const Key key = keyCodeFromName(ltrim(rtrim(line.substr(divider + 1))));
            if (it == std::end(buttonStrings) || key == 0)
            {
                LOG(Error) << "Invalid key in configuration file at Line " << line_no << std::endl;
                continue;
            }
            const auto i = std::distance(std::begin(buttonStrings), it);
            (state == Player1 ? p1 : p2)[i] = key;
        }
        else
            LOG(Error) << "Invalid line in key configuration at Line " << line_no << std::endl;

        ++line_no;
    }
}
}