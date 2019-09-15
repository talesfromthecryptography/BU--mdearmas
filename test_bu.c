#include <stdio.h>
#include <string.h>

#include "bu.h"


int main() {
  bigunsigned a,b,c,d,e,f;
  char s[BU_MAX_HEX+1];

  bu_readhex(&a,"CAB51AFFDEADBEEF");
  bu_readhex(&b,"111111111111");
  bu_cpy(&c, &b);

  /*bu_dbg_printf(&a);

  bu_shr_ip(&b, 8);

  bu_dbg_printf(&b);

  bu_add(&c, &b, &a);
  bu_dbg_printf(&c);

  bu_add_ip(&b, &a);
  bu_dbg_printf(&b);*/

  bu_mul_digit(&d, &c, 0x111111);
  bu_dbg_printf(&d);

  return 0;
}
