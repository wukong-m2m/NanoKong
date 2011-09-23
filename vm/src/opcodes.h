//
//  NanoVM, a tiny java VM for the Atmel AVR family
//  Copyright (C) 2005 by Till Harbaum <Till@Harbaum.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 

//
//  opcodes.h
//

#ifndef OPCODES_H
#define OPCODES_H

#define OP_NOP           0x00
#define OP_ACONST_NULL   0x01
#define OP_ICONST_M1     0x02
#define OP_ICONST_0      0x03
#define OP_ICONST_1      0x04
#define OP_ICONST_2      0x05
#define OP_ICONST_3      0x06
#define OP_ICONST_4      0x07
#define OP_ICONST_5      0x08
//missing OP_LCONST0			0x09
//missing OP_LCONST1			0x0a
#define OP_FCONST_0      0x0b // only if floating point compiled in
#define OP_FCONST_1      0x0c // only if floating point compiled in
#define OP_FCONST_2      0x0d // only if floating point compiled in
//missing OP_DCONST0			0x0e
//missing OP_DCONST1			0x0f
#define OP_BIPUSH        0x10
#define OP_SIPUSH        0x11
#define OP_LDC           0x12
//missing OP_LDC_W				0x13
//missing OP_LDC2_W				0x14
#define OP_ILOAD         0x15
//missing OP_LLOAD				0x16
#define OP_FLOAD         0x17 // only if floating point compiled in
//missing OP_DLOAD				0x18
//missing OP_ALOAD				0x19
#define OP_ILOAD_0       0x1a
#define OP_ILOAD_1       0x1b
#define OP_ILOAD_2       0x1c
#define OP_ILOAD_3       0x1d
//missing OP_LLOAD_0			0x1e
//missing OP_LLOAD_1			0x1f
//missing OP_LLOAD_2			0x20
//missing OP_LLOAD_3			0x21
#define OP_FLOAD_0       0x22 // only if floating point compiled in
#define OP_FLOAD_1       0x23 // only if floating point compiled in
#define OP_FLOAD_2       0x24 // only if floating point compiled in
#define OP_FLOAD_3       0x25 // only if floating point compiled in
//missing OP_DLOAD_0			0x26
//missing OP_DLOAD_1			0x27
//missing OP_DLOAD_2			0x28
//missing OP_DLOAD_3			0x29
#define OP_ALOAD_0       0x2a
#define OP_ALOAD_1       0x2b
#define OP_ALOAD_2       0x2c
#define OP_ALOAD_3       0x2d
#define OP_IALOAD        0x2e  // only if array compiled in
//missing OP_LALOAD				0x2f
#define OP_FALOAD        0x30  // only if array and floating point compiled in
//missing OP_DALOAD				0x31
#define OP_AALOAD        0x32  // only if array compiled in
#define OP_BALOAD        0x33  // only if array compiled in
//missing OP_CALOAD				0x34
//missing OP_SALOAD				0x35
#define OP_ISTORE        0x36
//missing OP_LSTORE				0x37
#define OP_FSTORE        0x38 // only if floating point compiled in
//missing OP_DSTORE				0x39
#define OP_ASTORE        0x3a
#define OP_ISTORE_0      0x3b
#define OP_ISTORE_1      0x3c
#define OP_ISTORE_2      0x3d
#define OP_ISTORE_3      0x3e
//missing OP_LSTORE_0			0x3f
//missing OP_LSTORE_1			0x40
//missing OP_LSTORE_2			0x41
//missing OP_LSTORE_3			0x42
#define OP_FSTORE_0      0x43 // only if floating point compiled in
#define OP_FSTORE_1      0x44 // only if floating point compiled in
#define OP_FSTORE_2      0x45 // only if floating point compiled in
#define OP_FSTORE_3      0x46 // only if floating point compiled in
//missing OP_DSTORE_0			0x47
//missing OP_DSTORE_1			0x48
//missing OP_DSTORE_2			0x49
//missing OP_DSTORE_3			0x4a
#define OP_ASTORE_0      0x4b
#define OP_ASTORE_1      0x4c
#define OP_ASTORE_2      0x4d
#define OP_ASTORE_3      0x4e
#define OP_IASTORE       0x4f  // only if array compiled in
//missing OP_LASTORE			0x50
#define OP_FASTORE       0x51  // only if array and floating point compiled in
//missing OP_DASTORE			0x52
#define OP_AASTORE       0x53  // only if array compiled in
#define OP_BASTORE       0x54  // only if array compiled in
//missing OP_CASTORE			0x55
//missing OP_SASTORE			0x56
#define OP_POP           0x57
#define OP_POP2          0x58
#define OP_DUP           0x59
#define OP_DUP_X1        0x5a  // only if extedend stack ops are compiled in
#define OP_DUP_X2        0x5b  // only if extedend stack ops are compiled in
#define OP_DUP2          0x5c
#define OP_DUP2_X1       0x5d  // only if extedend stack ops are compiled in
#define OP_DUP2_X2       0x5e  // only if extedend stack ops are compiled in
#define OP_SWAP          0x5f  // only if extedend stack ops are compiled in
#define OP_IADD          0x60
//missing OP_LADD				0x61
#define OP_FADD          0x62 // only if floating point compiled in
//missing OP_DADD				0x63
#define OP_ISUB          0x64
//missing OP_LSUB				0x65
#define OP_FSUB          0x66 // only if floating point compiled in
//missing OP_DSUB				0x67
#define OP_IMUL          0x68
//missing OP_LMUL				0x69
#define OP_FMUL          0x6a // only if floating point compiled in
//missing OP_DMUL				0x6b
#define OP_IDIV          0x6c
//missing OP_LDIV				0x6d
#define OP_FDIV          0x6e // only if floating point compiled in
//missing OP_DDIV				0x6f
#define OP_IREM          0x70
//missing OP_LREM				0x71
#define OP_FREM          0x72 // only if floating point compiled in
//missing OP_DREM				0x73
#define OP_INEG          0x74
//missing OP_LNEG				0x75
#define OP_FNEG          0x76 // only if floating point compiled in
//missing OP_DNEG				0x77
#define OP_ISHL          0x78
//missing OP_LSHL				0x79
#define OP_ISHR          0x7a
//missing OP_LSHR				0x7b
#define OP_IUSHR         0x7c
//missing OP_LUSHR				0x7d
#define OP_IAND          0x7e
//missing OP_LAND				0x7f
#define OP_IOR           0x80
//missing OP_LOR				0x81
#define OP_IXOR          0x82
//missing OP_LXOR				0x83
#define OP_IINC          0x84
//missing OP_I2L				0x85
#define OP_I2F           0x86 // only if floating point compiled in
//missing OP_I2D				0x87
//missing OP_L2I				0x88
//missing OP_L2F				0x89
//missing OP_L2D				0x8a
#define OP_F2I           0x8b // only if floating point compiled in
//missing OP_F2L				0x8c
//missing OP_F2D				0x8d
//missing OP_D2I				0x8e
//missing OP_D2L				0x8f
//missing OP_D2F				0x90
//missing OP_I2B				0x91
//missing OP_I2C				0x92
//missing OP_I2S				0x93
//missing OP_LCMP				0x94
#define OP_FCMPL         0x95 // only if floating point compiled in
#define OP_FCMPG         0x96 // only if floating point compiled in
//missing OP_DCMPL				0x97
//missing OP_DCMPG				0x98
#define OP_IFEQ          0x99
#define OP_IFNE          0x9a
#define OP_IFLT          0x9b
#define OP_IFGE          0x9c
#define OP_IFGT          0x9d
#define OP_IFLE          0x9e
#define OP_IF_ICMPEQ     0x9f
#define OP_IF_ICMPNE     0xa0
#define OP_IF_ICMPLT     0xa1
#define OP_IF_ICMPGE     0xa2
#define OP_IF_ICMPGT     0xa3
#define OP_IF_ICMPLE     0xa4
//missing OP_IF_ACMPEQ			0xa5
//missing OP_IF_ACMPNE			0xa6
#define OP_GOTO          0xa7
//missing OP_JSR				0xa8
//missing OP_RET				0xa9
#define OP_TABLESWITCH   0xaa
#define OP_LOOKUPSWITCH  0xab
#define OP_IRETURN       0xac
//missing OP_LRETURN			0xad
#define OP_FRETURN       0xae // only if floating point compiled in
//missing OP_DRETURN			0xaf
//missing OP_ARETURN			0xb0
#define OP_RETURN        0xb1
#define OP_GETSTATIC     0xb2
#define OP_PUTSTATIC     0xb3  // only if fields are compiled in
#define OP_GETFIELD      0xb4  // only if fields are compiled in
#define OP_PUTFIELD      0xb5  // only if fields are compiled in
#define OP_INVOKEVIRTUAL 0xb6
#define OP_INVOKESPECIAL 0xb7
#define OP_INVOKESTATIC  0xb8
//missing OP_INVOKEINTERFACE	0xb9
//missing OP_xxxunusedxxx		0xba		// this opcode is reserved "for historical reasons"
#define OP_NEW           0xbb
#define OP_NEWARRAY      0xbc  // only if array compiled in
#define OP_ANEWARRAY     0xbd  // only if array compiled in
#define OP_ARRAYLENGTH   0xbe  // only if array compiled in
//missing OP_ATHROW				0xbf
//missing OP_CHECKCAST			0xc0
//missing OP_INSTANCEOF			0xc1
//missing OP_MONITORENTER		0xc2
//missing OP_MONITOREXIT		0xc3
//missing OP_WIDE				0xc4
//missing OP_MULTIANEWARRAY		0xc5
//missing OP_IFNULL				0xc6
//missing OP_IFNONNULL			0xc7
//missing OP_GOTO_W				0xc8
//missing OP_JSR_W				0xc9
//missing OP_BREAKPOINT			0xca

// Opcodes 0xcb to 0xfd: reserved for future use

//missing OP_IMPDEP1			0xfe		// reserved for implementation-dependent operations within debuggers; should not appear in any class file
//missing OP_IMPDEP2			0xff		// reserved for implementation-dependent operations within debuggers; should not appear in any class file

#endif // OPCODES_H
