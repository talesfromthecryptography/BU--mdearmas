#include <stdio.h>
#include <string.h>

#include "bu.h"


int main() {
  bigunsigned a,b,c,d,e,f,g,h;
  char s[BU_MAX_HEX+1];

  bu_readhex(&a,"CAB51AFFDEADBEEF");
  bu_readhex(&b,"111111111111");
  bu_cpy(&c, &b);
  bu_cpy(&e, &b);
  bu_cpy(&f, &b);
  bu_cpy(&g, &a);

  bu_dbg_printf(&a);

  bu_shr_ip(&b, 8);

  bu_dbg_printf(&b);

  bu_add(&c, &b, &a);
  bu_dbg_printf(&c);

  bu_add_ip(&e, &a);
  bu_dbg_printf(&e);

  bu_mul_digit_ip(&f, 0x1111);
  bu_dbg_printf(&f);

  bu_mul(&d, &a, &b);
  bu_dbg_printf(&d);

  bu_mul_ip(&a, &b);
  bu_dbg_printf(&a);

  bu_sqr(&h, &g);
  bu_dbg_printf(&h);

  bu_sqr_ip(&g);
  bu_dbg_printf(&g);

  return 0;
}
