#source: load1.s
#as: --64
#ld: -melf_x86_64
#objdump: -dw --sym
#target: x86_64-*-nacl*

.*: +file format .*

SYMBOL TABLE:
#...
0+100300c8 l     O .data	0+1 bar
#...
0+100300c9 g     O .data	0+1 foo
#...

Disassembly of section .text:

0+20000 <_start>:
[ 	]*[a-f0-9]+:	81 d0 c8 00 03 10    	adc    \$0x100300c8,%eax
[ 	]*[a-f0-9]+:	81 c3 c8 00 03 10    	add    \$0x100300c8,%ebx
[ 	]*[a-f0-9]+:	81 e1 c8 00 03 10    	and    \$0x100300c8,%ecx
[ 	]*[a-f0-9]+:	81 fa c8 00 03 10    	cmp    \$0x100300c8,%edx
[ 	]*[a-f0-9]+:	81 ce c8 00 03 10    	or     \$0x100300c8,%esi
[ 	]*[a-f0-9]+:	81 df c8 00 03 10    	sbb    \$0x100300c8,%edi
[ 	]*[a-f0-9]+:	81 ed c8 00 03 10    	sub    \$0x100300c8,%ebp
[ 	]*[a-f0-9]+:	41 81 f0 c8 00 03 10 	xor    \$0x100300c8,%r8d
[ 	]*[a-f0-9]+:	41 f7 c7 c8 00 03 10 	test   \$0x100300c8,%r15d
[ 	]*[a-f0-9]+:	48 81 d0 c8 00 03 10 	adc    \$0x100300c8,%rax
[ 	]*[a-f0-9]+:	48 81 c3 c8 00 03 10 	add    \$0x100300c8,%rbx
[ 	]*[a-f0-9]+:	48 81 e1 c8 00 03 10 	and    \$0x100300c8,%rcx
[ 	]*[a-f0-9]+:	48 81 fa c8 00 03 10 	cmp    \$0x100300c8,%rdx
[ 	]*[a-f0-9]+:	48 81 cf c8 00 03 10 	or     \$0x100300c8,%rdi
[ 	]*[a-f0-9]+:	48 81 de c8 00 03 10 	sbb    \$0x100300c8,%rsi
[ 	]*[a-f0-9]+:	48 81 ed c8 00 03 10 	sub    \$0x100300c8,%rbp
[ 	]*[a-f0-9]+:	49 81 f0 c8 00 03 10 	xor    \$0x100300c8,%r8
[ 	]*[a-f0-9]+:	49 f7 c7 c8 00 03 10 	test   \$0x100300c8,%r15
[ 	]*[a-f0-9]+:	81 d0 c9 00 03 10    	adc    \$0x100300c9,%eax
[ 	]*[a-f0-9]+:	81 c3 c9 00 03 10    	add    \$0x100300c9,%ebx
[ 	]*[a-f0-9]+:	81 e1 c9 00 03 10    	and    \$0x100300c9,%ecx
[ 	]*[a-f0-9]+:	81 fa c9 00 03 10    	cmp    \$0x100300c9,%edx
[ 	]*[a-f0-9]+:	81 ce c9 00 03 10    	or     \$0x100300c9,%esi
[ 	]*[a-f0-9]+:	81 df c9 00 03 10    	sbb    \$0x100300c9,%edi
[ 	]*[a-f0-9]+:	81 ed c9 00 03 10    	sub    \$0x100300c9,%ebp
[ 	]*[a-f0-9]+:	41 81 f0 c9 00 03 10 	xor    \$0x100300c9,%r8d
[ 	]*[a-f0-9]+:	41 f7 c7 c9 00 03 10 	test   \$0x100300c9,%r15d
[ 	]*[a-f0-9]+:	48 81 d0 c9 00 03 10 	adc    \$0x100300c9,%rax
[ 	]*[a-f0-9]+:	48 81 c3 c9 00 03 10 	add    \$0x100300c9,%rbx
[ 	]*[a-f0-9]+:	48 81 e1 c9 00 03 10 	and    \$0x100300c9,%rcx
[ 	]*[a-f0-9]+:	48 81 fa c9 00 03 10 	cmp    \$0x100300c9,%rdx
[ 	]*[a-f0-9]+:	48 81 cf c9 00 03 10 	or     \$0x100300c9,%rdi
[ 	]*[a-f0-9]+:	48 81 de c9 00 03 10 	sbb    \$0x100300c9,%rsi
[ 	]*[a-f0-9]+:	48 81 ed c9 00 03 10 	sub    \$0x100300c9,%rbp
[ 	]*[a-f0-9]+:	49 81 f0 c9 00 03 10 	xor    \$0x100300c9,%r8
[ 	]*[a-f0-9]+:	49 f7 c7 c9 00 03 10 	test   \$0x100300c9,%r15
#pass
