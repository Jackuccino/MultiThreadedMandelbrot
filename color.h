#include <string>
using std::string;
#include <sstream>
using std::stringstream;
#include <iostream>
using std::hex;

class Color
{
private:
    int red;
    int green;
    int blue;
public:
    Color(string hex)
    {
        int rgb[3];
        stringstream ss;
        string str;

        // Drop a hash if the value has one
        if (hex[0] == '#')
            hex.erase(0, 1);

        for (int i = 0; i < 3; i++)
        {
            str = hex.substr(i * 2, 2);
            ss << std::hex << str;
            ss >> rgb[i];
            ss.clear();
        }

        red = rgb[0];
        green = rgb[1];
        blue = rgb[2];
    }
    Color(const Color &obj)
    {
        red = obj.red;
        green = obj.green;
        blue = obj.blue;
    }
    ~Color() { }
    int R() { return red; }
    int G() { return green; }
    int B() { return blue; }
};