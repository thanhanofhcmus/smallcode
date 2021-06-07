// #pragma once

#include <vector>
#include <string>

enum class Color : char {
    Black = 30,
    Red,
    Green,
    Orange,
    Blue,
    Purple,
    Cyan,
    White,

    DarkBlack = 40,
    DarkRed,
    DarkGreen,
    DarkOrange,
    DarkBlue,
    DarkPurple,
    DarkCyan,
    DarkWhite,

    Default = White,
};

inline Color operator++(Color& c) {
    if (c == Color::White)
        c = Color::DarkBlack;
    else if (c == Color::DarkWhite)
        c = Color::Black;
    else
        c = static_cast<Color>(static_cast<std::underlying_type_t<Color>>(c) + 1);
    return c;
}

enum class Attribute : char {
    None = 0,
    Bold = 1,
    Underline = 4,
    Inverse = 7,

    Default = None,
};

inline Attribute operator++(Attribute& a) {
    switch (a) {
    case Attribute::None:      a = Attribute::Bold     ; break;
    case Attribute::Bold:      a = Attribute::Underline; break;
    case Attribute::Underline: a = Attribute::Inverse  ; break;
    case Attribute::Inverse:   a = Attribute::None     ; break;
    }
    return a;
}

struct Pixel {
    char glyph;
    Color color;
    Attribute att;

    Pixel()
        : glyph(' '), color(Color::Default), att(Attribute::None) {}

    explicit Pixel(char c)
        : glyph(c), color(Color::Default), att(Attribute::None) {}

    explicit Pixel(char c, Color col)
        : glyph(c), color(col), att(Attribute::None) {}

    Pixel(char c, Color col, Attribute a)
        : glyph(c), color(col), att(a) {}
};

using PixelRow = std::vector<Pixel>;

PixelRow StringToPixelRow(std::string const& str, Color color = Color::Default, Attribute att = Attribute::Default);

std::string ConstructColoredString(Pixel pixel);
std::string ConstructColoredString(PixelRow const& row);

class Renderer {
private:

    static uint16_t mOldConsoleMode;
    static size_t mContextWidth;
    static size_t mContextHeight;
    static std::vector<PixelRow> mBaseContext;
    static std::vector<PixelRow> mContext;

public:

    static void Init();

    static void ShutDown();

    static size_t Width();
    static size_t Height();

    static void Submit(int x, int h, Pixel pixel);
    static void Submit(int x, int h, PixelRow const& row);
    static void Submit(int x, int h, char c, Color color = Color::Default, Attribute att = Attribute::Default);
    static void Submit(int x, int h, std::string const& str, Color color = Color::Default, Attribute att = Attribute::Default);

    static void Clear(Color color = Color::Default);
    static void Flush();

private:

    static void ResizeContext(size_t width, size_t height);
};

#ifdef SIMPLE_COLORED_RENDERER_IMPLEMENTATION
#include <iostream>

PixelRow StringToPixelRow(std::string const& str, Color color, Attribute att) {
    PixelRow row(str.size());
    for (int i = 0; i < str.size(); ++i)
        row[i] = Pixel{ str[i], color, att };
    return row;
}

std::string ConstructColoredString(Pixel pixel) {
    auto const pre = std::string("\033[");
    auto const col = std::to_string(static_cast<char>(pixel.color));
    auto const att = std::to_string(static_cast<char>(pixel.att));

    return pre + att + ';' + col + 'm' + pixel.glyph + "\033[0m";
}

std::string ConstructColoredString(PixelRow const& row) {
    std::string s;
    for (Pixel p : row)
        s += ConstructColoredString(p);
    return s;
}

uint16_t Renderer::mOldConsoleMode = 0;
size_t Renderer::mContextWidth = 0;
size_t Renderer::mContextHeight = 0;
std::vector<PixelRow> Renderer::mBaseContext{};
std::vector<PixelRow> Renderer::mContext{};

#if defined(WIN32) || defined (_WIN32) || defined (__WIN32__) || defined (__CYGWIN__)

#include <Windows.h>

void Renderer::Init() {
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mOldConsoleMode);
    WINDOW_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &nfo);
    size_t width = info.srWindow.Right - info.srWindow.Left + 1;
    size_t height = info.srWindow.Bottom - info.srWindow.Top;
    ResizeContext(width, height);
}

void Renderer::ShutDown() {
    FlushConsoleInputBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), mOldConsoleMode);
}
# elif defined (__linux) || (__linux__)

#include <sys/ioctl.h>

void Renderer::Init() {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    ResizeContext(w.ws_col, w.ws_row - 1);
}

void Renderer::ShutDown() {
}

#endif

inline bool IsOutside(int a, int b) {
    return a < 0 || a >= b;
}

size_t Renderer::Width() {
    return mContextWidth;
}

size_t Renderer::Height() {
    return mContextHeight;
}

void Renderer::Submit(int x, int y, Pixel pixel) {
    if (x < 0 || x >= mContextWidth  ||
        y < 0 || y >= mContextHeight)
        return;
    mContext[y][x] = pixel;
}

void Renderer::Submit(int x, int y, PixelRow const& row) {
    if (y < 0 || y >= mContextHeight)
        return;
    for (Pixel p : row)
        Submit( x++, y, p);
}

void Renderer::Submit(int x, int y, char c, Color color, Attribute att) {
    Renderer::Submit(x, y, Pixel{ c, color, att });
}

void Renderer::Submit(int x, int y, std::string const& str, Color color, Attribute att) {
    Submit(x, y, StringToPixelRow(str, color, att));
}

void Renderer::Clear(Color color) {
    for (PixelRow& row : mBaseContext)
        for (Pixel& pixel : row)
            pixel = Pixel{ ' ', color };
    mContext = mBaseContext;
}

void Renderer::Flush() {
    std::cout << "\033[H" << std::flush;\
    std::string s;
    for (auto const& row : mContext)
        s += ConstructColoredString(row) + '\n';
    std::cout << s << std::flush;
}

void Renderer::ResizeContext(size_t width, size_t height) {
    mBaseContext = std::vector<PixelRow>(height, PixelRow(width, Pixel(' ')));
    mContext = mBaseContext;
    mContextWidth = width;
    mContextHeight = height;
}

#endif // SIMPLE_COLORED_RENDERER_IMPLEMENTATION
