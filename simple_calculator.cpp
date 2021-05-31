#include <iostream>
#include <cmath>

/*
Based on recursive decent parser technique
Barkus-Naur fomr:
expr    -> term
term    -> factor (("+" | "-") factor)*
factor  -> expo   (("*" | "/") expo)*
expo    -> unary  ("^" unary)?
unary   -> "-" unary | primary
primary -> NUMBER | "(" term ")"
*/

using namespace std;

string src;
int i = 0; // index of the currently examined char
bool had_error = false;

float term();

// Move the index to the next non-whiespace char and check
// if it matchs the expected char or not.
bool match(char expected) {
    while (i < src.length() && src[i] == ' ') ++i;
    return src[i] != expected ? false : ++i;
}

float primary() {
    float val = 0;

    if (match('(')) {
        val = term();
        if (!match(')')) {
            i = src.length() - 1;
            had_error = true;
        }
    }
    else if (isdigit(src[i])) {
        val = stof(src.substr(i));
        // Move i to the first char after the literal.
        i = src.find_first_not_of("0123456789.", i);
    } else {
        i = src.length() - 1;
        had_error = true;
    }

    return val;
}

float unary() { return match('-') ? -unary() : primary(); }

float expo() {
    float val = unary();
    if (match('^')) val = pow(val, unary());
    return val;
}

float factor() {
    float val = expo();
    while (match('*')) val *= expo();
    while (match('/')) val /= expo();
    return val;
}

float term() {
    float val = factor();
    while (match('+')) val += factor();
    while (match('-')) val -= factor();
    return val;
}

int main() {
    while (1) {
        cout << "> ";
        getline(cin, src);
        if (src.empty()) continue;

        i = 0, had_error = false;
        float val = term();

        if (had_error) cerr << "Could not evaluate expression\n";
        else cout << val << '\n';
    }
}
