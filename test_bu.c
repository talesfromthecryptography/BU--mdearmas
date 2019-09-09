#include <stdio.h>
#include <string.h>

#include "bu.h"


int main() {
  bigunsigned a,b,c;
  char s[BU_MAX_HEX+1];

  bu_readhex(&a,"CAB51AFFBEEF");
  bu_readhex(&b,"111111111111");
  //bu_cpy(&c, &a);

  bu_dbg_printf(&a);
  bu_dbg_printf(&b);

  //bu_add(&c, &a, &b);

  //bu_dbg_printf(&c);
  return 0;
}
