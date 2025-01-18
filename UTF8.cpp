#include <string>
#include <cstdio>
#include <fstream>

#ifndef _WIN32
#define _CRT_INSECURE_DEPRECATE(_Replacement)
#endif 
namespace utf8
{
    class converter
    {
    public:
        static std::wstring utf8_to_wstring(const std::string& str)
        {
            std::wstring result;
            result.reserve(str.size());

            for (size_t i = 0; i < str.size();)
            {
                unsigned char c = str[i];
                if (c < 0x80)
                {
                    result.push_back(c);
                    i += 1;
                }
                else if ((c & 0xE0) == 0xC0)
                {
                    wchar_t ch = ((str[i] & 0x1F) << 6) | (str[i + 1] & 0x3F);
                    result.push_back(ch);
                    i += 2;
                }
                else if ((c & 0xF0) == 0xE0)
                {
                    wchar_t ch = ((str[i] & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
                    result.push_back(ch);
                    i += 3;
                }
                else if ((c & 0xF8) == 0xF0)
                {
                    wchar_t ch = ((str[i] & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F);
                    result.push_back(ch);
                    i += 4;
                }
            }
            return result;
        }

        static std::string wstring_to_utf8(const std::wstring& wstr)
        {
            std::string result;
            result.reserve(wstr.size() * 2);

            for (wchar_t wc : wstr)
            {
                if (wc < 0x80)
                {
                    result.push_back(static_cast<char>(wc));
                }
                else if (wc < 0x800)
                {
                    result.push_back(static_cast<char>((wc >> 6) | 0xC0));
                    result.push_back(static_cast<char>((wc & 0x3F) | 0x80));
                }
                else
                {
                    result.push_back(static_cast<char>((wc >> 12) | 0xE0));
                    result.push_back(static_cast<char>(((wc >> 6) & 0x3F) | 0x80));
                    result.push_back(static_cast<char>((wc & 0x3F) | 0x80));
                }
            }
            return result;
        }
    };

    class fstream : public std::fstream, protected converter
    {
    public:
        fstream() : std::fstream() {}

        explicit fstream(const std::string& utf8_path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        {
            open(utf8_path, mode);
        }

        void open(const std::string& utf8_path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
        {
#ifdef _WIN32
            auto wide_path = utf8_to_wstring(utf8_path);
            std::fstream::open(wide_path.c_str(), mode);
#else
            std::fstream::open(utf8_path, mode);
#endif
        }
    };

    class ifstream : public std::ifstream, protected converter
    {
    public:
        ifstream() : std::ifstream() {}

        explicit ifstream(const std::string& utf8_path, std::ios_base::openmode mode = std::ios_base::in) { open(utf8_path, mode); }

        void open(const std::string& utf8_path, std::ios_base::openmode mode = std::ios_base::in)
        {
#ifdef _WIN32
            auto wide_path = utf8_to_wstring(utf8_path);
            std::ifstream::open(wide_path.c_str(), mode);
#else
            std::ifstream::open(utf8_path, mode);
#endif
        }
    };

    class ofstream : public std::ofstream, protected converter
    {
    public:
        ofstream() : std::ofstream() {}

        explicit ofstream(const std::string& utf8_path, std::ios_base::openmode mode = std::ios_base::out) { open(utf8_path, mode); }

        void open(const std::string& utf8_path, std::ios_base::openmode mode = std::ios_base::out)
        {
#ifdef _WIN32
            auto wide_path = utf8_to_wstring(utf8_path);
            std::ofstream::open(wide_path.c_str(), mode);
#else
            std::ofstream::open(utf8_path, mode);
#endif
        }
    };

    _CRT_INSECURE_DEPRECATE(utf8::fopen_s)
        inline FILE* fopen(const char* path, const char* mode)
    {
#ifdef _WIN32
        auto wide_path = converter::utf8_to_wstring(path);
        auto wide_mode = converter::utf8_to_wstring(mode);
#ifdef _CRT_INSECURE_DEPRECATE
        FILE* f = nullptr;
        _wfopen_s(&f, wide_path.c_str(), wide_mode.c_str());
        return f;
#else
        return _wfopen(wide_path.c_str(), wide_mode.c_str());
#endif

#else
        return ::fopen(path, mode);
#endif
    }

    _CRT_INSECURE_DEPRECATE(utf8::freopen_s)
        inline FILE* freopen(const char* path, const char* mode, FILE* stream)
    {
#ifdef _WIN32
        auto wide_path = converter::utf8_to_wstring(path);
        auto wide_mode = converter::utf8_to_wstring(mode);
#ifdef _CRT_INSECURE_DEPRECATE
        FILE* f = nullptr;
        _wfreopen_s(&f, wide_path.c_str(), wide_mode.c_str(), stream);
        return f;
#else
        return _wfreopen(wide_path.c_str(), wide_mode.c_str(), stream);
#endif
#else
        return ::freopen(path, mode, stream);
#endif
    }

    inline int rename(const char* old_filename, const char* new_filename)
    {
#ifdef _WIN32
        auto wide_oldpath = converter::utf8_to_wstring(old_filename);
        auto wide_newpath = converter::utf8_to_wstring(new_filename);
        return _wrename(wide_oldpath.c_str(), wide_newpath.c_str());
#else
        return ::rename(old_filename, new_filename);
#endif
    }

    inline int remove(const char* pathname)
    {
#ifdef _WIN32
        auto wide_path = converter::utf8_to_wstring(pathname);
        return _wremove(wide_path.c_str());
#else
        return ::remove(pathname);
#endif
    }

    //***************************************************
    //Windows only functions
#if defined(_WIN32) 
    FILE* _fsopen(const char* filename, const char* mode, int shflag)
    {
        auto wide_path = converter::utf8_to_wstring(filename);
        auto wide_mode = converter::utf8_to_wstring(mode);
        return _wfsopen(wide_path.c_str(), wide_mode.c_str(), shflag);
    }

#if defined(_CRT_INSECURE_DEPRECATE)
    inline errno_t fopen_s(FILE** stream, const char* path, const char* mode)
    {
        auto wide_path = converter::utf8_to_wstring(path);
        auto wide_mode = converter::utf8_to_wstring(mode);
        return _wfopen_s(stream, wide_path.c_str(), wide_mode.c_str());
    }

    inline errno_t freopen_s(FILE** stream, const char* path, const char* mode, FILE* oldstream)
    {
        auto wide_path = converter::utf8_to_wstring(path);
        auto wide_mode = converter::utf8_to_wstring(mode);
        return _wfreopen_s(stream, wide_path.c_str(), wide_mode.c_str(), oldstream);
    }
#endif /*_CRT_INSECURE_DEPRECATE*/
#endif /*_WIN32*/
    //***************************************************
} // namespace utf8


#include <iostream>
int main()
{
    std::string fileName = "\xE3\x83\x86\xE3\x82\xB9\xE3\x83\x88.txt"; //テスト.txt
    utf8::ofstream out(fileName);
    out << "Hello World" << std::endl;
    out.close();

    std::string text;
    utf8::ifstream in(fileName);
    std::getline(in, text);
    in >> text;
    in.close();

    std::cout << text << std::endl;

    FILE* f = utf8::fopen(fileName.c_str(), "r");
    char line[256];
    if (f != nullptr)
    {
        while (fgets(line, sizeof(line), f))
        {
            std::cout << line << std::endl;
        }
        fclose(f);
    }

    return 0;
}
