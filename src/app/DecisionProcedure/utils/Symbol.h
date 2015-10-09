//
// Created by Raph on 09/10/2015.
//

#ifndef WSKS_SYMBOL_H
#define WSKS_SYMBOL_H


class Symbol {
public:
using Vars = std::vector<size_t>;

private:
    std::string _track;

public:
    void ProjectVars(Vars freeVars);
    bool IsEmpty();
};

#endif //WSKS_SYMBOL_H
