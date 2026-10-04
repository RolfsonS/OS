#ifndef E1000_H
#define E1000_H

void test();



/* Write a value to a register.
 * This will add onto the BAR.
 *
 * Param: uint32_t reg, The offset register to write to.
 * Param: uint32_t val, The value to write to this register.
 */
void write_reg(uint32_t reg, uint32_t val);

/* Read a value from a register.
 *
 * Param: uint32_t reg, The register to read from.
 *
 * Return: uint32_t, Value read from register.
 */
uint32_t read_reg(uint32_t reg);































#endif 
