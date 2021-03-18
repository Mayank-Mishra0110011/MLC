#ifndef MLC_VALUE_H
#define MLC_VALUE_H

#include "common.h"
#include "object.h"

void initVal(ValArr *);
void writeVal(ValArr *, Value);
void deleteVal(ValArr *);
void printVal(Value val);

bool isEqual(Value, Value);

#endif