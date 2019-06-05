#include "enableArrows.h"

static int enableLeft = 1;
static int enableRight = 1;
static int enableTop = 1;
static int enableBottom = 1;

void enableLeftArrow(const int enable) {enableLeft = enable;}

void enableRightArrow(const int enable) {enableRight = enable;}

void enableTopArrow(const int enable) {enableTop = enable;}

void enableBottomArrow(const int enable) {enableBottom = enable;}

int isLeftArrowEnabled() {return enableLeft;}

int isRightArrowEnabled() {return enableRight;}

int isTopArrowEnabled() {return enableTop;}

int isBottomArrowEnabled() {return enableBottom;}
