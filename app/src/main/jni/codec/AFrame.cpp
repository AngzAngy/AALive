//
// Created by Administrator on 2021/11/5.
//
#include "AFrame.h"

AFrame* AFrame::allocFrame() {
    AFrame *pFrame = new AFrame;
    return pFrame;
}

void AFrame::freeFrame(AFrame* pFrame) {
    if(pFrame) {
        pFrame->free();
        delete pFrame;
    }
}

AFrame::AFrame():type(UnknownFrame){

}