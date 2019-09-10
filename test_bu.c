#include <stdio.h>
#include <string.h>

#include "bu.h"


int main() {
  bigunsigned a,b,c,d,e,f;
  char s[BU_MAX_HEX+1];

  bu_readhex(&a,"CAB51AFFDEADBEEF");
  bu_readhex(&b,"111111111111");
  bu_cpy(&c, &a);
  bu_cpy(&e, &b);

  bu_dbg_printf(&a);

  bu_shr_ip(&a, 36);
  bu_dbg_printf(&a);

  bu_shr(&d, &c, 36);
  bu_dbg_printf(&d);

  bu_shl_ip(&b, 36);
  bu_dbg_printf(&b);

  bu_shl(&f, &e, 36);
  bu_dbg_printf(&f);
  bu_shr_ip(&f, 32);
  bu_dbg_printf(&f);

  //bu_add(&c, &a, &b);

  //bu_dbg_printf(&c);
  return 0;
}
