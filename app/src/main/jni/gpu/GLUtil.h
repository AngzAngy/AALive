//
// Created on 2017/10/26.
//

#include<string>

#ifndef AALIVE_GLUTIL_H
#define AALIVE_GLUTIL_H

class GLUtil{
public:
    static void checkGLError(const std::string& msg);
    static void checkGLError(const char* msg);
};
#endif //AALIVE_GLUTIL_H
