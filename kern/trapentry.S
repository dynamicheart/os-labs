/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(num)						\
	.globl vector ## num;		/* define global symbol for 'name' */	\
	.type vector ## num, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	vector ## num:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(num)					\
	.globl vector ## num;							\
	.type vector ## num, @function;						\
	.align 2;							\
	vector ## num:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(0)
TRAPHANDLER_NOEC(1)
TRAPHANDLER_NOEC(2)
TRAPHANDLER_NOEC(3)
TRAPHANDLER_NOEC(4)
TRAPHANDLER_NOEC(5)
TRAPHANDLER_NOEC(6)
TRAPHANDLER_NOEC(7)
TRAPHANDLER(8)
TRAPHANDLER_NOEC(9)
TRAPHANDLER(10)
TRAPHANDLER(11)
TRAPHANDLER(12)
TRAPHANDLER(13)
TRAPHANDLER(14)
TRAPHANDLER_NOEC(15)
TRAPHANDLER_NOEC(16)
TRAPHANDLER(17)
TRAPHANDLER_NOEC(18)
TRAPHANDLER_NOEC(19)
TRAPHANDLER_NOEC(20)
TRAPHANDLER_NOEC(21)
TRAPHANDLER_NOEC(22)
TRAPHANDLER_NOEC(23)
TRAPHANDLER_NOEC(24)
TRAPHANDLER_NOEC(25)
TRAPHANDLER_NOEC(26)
TRAPHANDLER_NOEC(27)
TRAPHANDLER_NOEC(28)
TRAPHANDLER_NOEC(29)
TRAPHANDLER_NOEC(30)
TRAPHANDLER_NOEC(31)
TRAPHANDLER_NOEC(32)
TRAPHANDLER_NOEC(33)
TRAPHANDLER_NOEC(34)
TRAPHANDLER_NOEC(35)
TRAPHANDLER_NOEC(36)
TRAPHANDLER_NOEC(37)
TRAPHANDLER_NOEC(38)
TRAPHANDLER_NOEC(39)
TRAPHANDLER_NOEC(40)
TRAPHANDLER_NOEC(41)
TRAPHANDLER_NOEC(42)
TRAPHANDLER_NOEC(43)
TRAPHANDLER_NOEC(44)
TRAPHANDLER_NOEC(45)
TRAPHANDLER_NOEC(46)
TRAPHANDLER_NOEC(47)
TRAPHANDLER_NOEC(48)
TRAPHANDLER_NOEC(49)
TRAPHANDLER_NOEC(50)
TRAPHANDLER_NOEC(51)
TRAPHANDLER_NOEC(52)
TRAPHANDLER_NOEC(53)
TRAPHANDLER_NOEC(54)
TRAPHANDLER_NOEC(55)
TRAPHANDLER_NOEC(56)
TRAPHANDLER_NOEC(57)
TRAPHANDLER_NOEC(58)
TRAPHANDLER_NOEC(59)
TRAPHANDLER_NOEC(60)
TRAPHANDLER_NOEC(61)
TRAPHANDLER_NOEC(62)
TRAPHANDLER_NOEC(63)
TRAPHANDLER_NOEC(64)
TRAPHANDLER_NOEC(65)
TRAPHANDLER_NOEC(66)
TRAPHANDLER_NOEC(67)
TRAPHANDLER_NOEC(68)
TRAPHANDLER_NOEC(69)
TRAPHANDLER_NOEC(70)
TRAPHANDLER_NOEC(71)
TRAPHANDLER_NOEC(72)
TRAPHANDLER_NOEC(73)
TRAPHANDLER_NOEC(74)
TRAPHANDLER_NOEC(75)
TRAPHANDLER_NOEC(76)
TRAPHANDLER_NOEC(77)
TRAPHANDLER_NOEC(78)
TRAPHANDLER_NOEC(79)
TRAPHANDLER_NOEC(80)
TRAPHANDLER_NOEC(81)
TRAPHANDLER_NOEC(82)
TRAPHANDLER_NOEC(83)
TRAPHANDLER_NOEC(84)
TRAPHANDLER_NOEC(85)
TRAPHANDLER_NOEC(86)
TRAPHANDLER_NOEC(87)
TRAPHANDLER_NOEC(88)
TRAPHANDLER_NOEC(89)
TRAPHANDLER_NOEC(90)
TRAPHANDLER_NOEC(91)
TRAPHANDLER_NOEC(92)
TRAPHANDLER_NOEC(93)
TRAPHANDLER_NOEC(94)
TRAPHANDLER_NOEC(95)
TRAPHANDLER_NOEC(96)
TRAPHANDLER_NOEC(97)
TRAPHANDLER_NOEC(98)
TRAPHANDLER_NOEC(99)
TRAPHANDLER_NOEC(100)
TRAPHANDLER_NOEC(101)
TRAPHANDLER_NOEC(102)
TRAPHANDLER_NOEC(103)
TRAPHANDLER_NOEC(104)
TRAPHANDLER_NOEC(105)
TRAPHANDLER_NOEC(106)
TRAPHANDLER_NOEC(107)
TRAPHANDLER_NOEC(108)
TRAPHANDLER_NOEC(109)
TRAPHANDLER_NOEC(110)
TRAPHANDLER_NOEC(111)
TRAPHANDLER_NOEC(112)
TRAPHANDLER_NOEC(113)
TRAPHANDLER_NOEC(114)
TRAPHANDLER_NOEC(115)
TRAPHANDLER_NOEC(116)
TRAPHANDLER_NOEC(117)
TRAPHANDLER_NOEC(118)
TRAPHANDLER_NOEC(119)
TRAPHANDLER_NOEC(120)
TRAPHANDLER_NOEC(121)
TRAPHANDLER_NOEC(122)
TRAPHANDLER_NOEC(123)
TRAPHANDLER_NOEC(124)
TRAPHANDLER_NOEC(125)
TRAPHANDLER_NOEC(126)
TRAPHANDLER_NOEC(127)
TRAPHANDLER_NOEC(128)
TRAPHANDLER_NOEC(129)
TRAPHANDLER_NOEC(130)
TRAPHANDLER_NOEC(131)
TRAPHANDLER_NOEC(132)
TRAPHANDLER_NOEC(133)
TRAPHANDLER_NOEC(134)
TRAPHANDLER_NOEC(135)
TRAPHANDLER_NOEC(136)
TRAPHANDLER_NOEC(137)
TRAPHANDLER_NOEC(138)
TRAPHANDLER_NOEC(139)
TRAPHANDLER_NOEC(140)
TRAPHANDLER_NOEC(141)
TRAPHANDLER_NOEC(142)
TRAPHANDLER_NOEC(143)
TRAPHANDLER_NOEC(144)
TRAPHANDLER_NOEC(145)
TRAPHANDLER_NOEC(146)
TRAPHANDLER_NOEC(147)
TRAPHANDLER_NOEC(148)
TRAPHANDLER_NOEC(149)
TRAPHANDLER_NOEC(150)
TRAPHANDLER_NOEC(151)
TRAPHANDLER_NOEC(152)
TRAPHANDLER_NOEC(153)
TRAPHANDLER_NOEC(154)
TRAPHANDLER_NOEC(155)
TRAPHANDLER_NOEC(156)
TRAPHANDLER_NOEC(157)
TRAPHANDLER_NOEC(158)
TRAPHANDLER_NOEC(159)
TRAPHANDLER_NOEC(160)
TRAPHANDLER_NOEC(161)
TRAPHANDLER_NOEC(162)
TRAPHANDLER_NOEC(163)
TRAPHANDLER_NOEC(164)
TRAPHANDLER_NOEC(165)
TRAPHANDLER_NOEC(166)
TRAPHANDLER_NOEC(167)
TRAPHANDLER_NOEC(168)
TRAPHANDLER_NOEC(169)
TRAPHANDLER_NOEC(170)
TRAPHANDLER_NOEC(171)
TRAPHANDLER_NOEC(172)
TRAPHANDLER_NOEC(173)
TRAPHANDLER_NOEC(174)
TRAPHANDLER_NOEC(175)
TRAPHANDLER_NOEC(176)
TRAPHANDLER_NOEC(177)
TRAPHANDLER_NOEC(178)
TRAPHANDLER_NOEC(179)
TRAPHANDLER_NOEC(180)
TRAPHANDLER_NOEC(181)
TRAPHANDLER_NOEC(182)
TRAPHANDLER_NOEC(183)
TRAPHANDLER_NOEC(184)
TRAPHANDLER_NOEC(185)
TRAPHANDLER_NOEC(186)
TRAPHANDLER_NOEC(187)
TRAPHANDLER_NOEC(188)
TRAPHANDLER_NOEC(189)
TRAPHANDLER_NOEC(190)
TRAPHANDLER_NOEC(191)
TRAPHANDLER_NOEC(192)
TRAPHANDLER_NOEC(193)
TRAPHANDLER_NOEC(194)
TRAPHANDLER_NOEC(195)
TRAPHANDLER_NOEC(196)
TRAPHANDLER_NOEC(197)
TRAPHANDLER_NOEC(198)
TRAPHANDLER_NOEC(199)
TRAPHANDLER_NOEC(200)
TRAPHANDLER_NOEC(201)
TRAPHANDLER_NOEC(202)
TRAPHANDLER_NOEC(203)
TRAPHANDLER_NOEC(204)
TRAPHANDLER_NOEC(205)
TRAPHANDLER_NOEC(206)
TRAPHANDLER_NOEC(207)
TRAPHANDLER_NOEC(208)
TRAPHANDLER_NOEC(209)
TRAPHANDLER_NOEC(210)
TRAPHANDLER_NOEC(211)
TRAPHANDLER_NOEC(212)
TRAPHANDLER_NOEC(213)
TRAPHANDLER_NOEC(214)
TRAPHANDLER_NOEC(215)
TRAPHANDLER_NOEC(216)
TRAPHANDLER_NOEC(217)
TRAPHANDLER_NOEC(218)
TRAPHANDLER_NOEC(219)
TRAPHANDLER_NOEC(220)
TRAPHANDLER_NOEC(221)
TRAPHANDLER_NOEC(222)
TRAPHANDLER_NOEC(223)
TRAPHANDLER_NOEC(224)
TRAPHANDLER_NOEC(225)
TRAPHANDLER_NOEC(226)
TRAPHANDLER_NOEC(227)
TRAPHANDLER_NOEC(228)
TRAPHANDLER_NOEC(229)
TRAPHANDLER_NOEC(230)
TRAPHANDLER_NOEC(231)
TRAPHANDLER_NOEC(232)
TRAPHANDLER_NOEC(233)
TRAPHANDLER_NOEC(234)
TRAPHANDLER_NOEC(235)
TRAPHANDLER_NOEC(236)
TRAPHANDLER_NOEC(237)
TRAPHANDLER_NOEC(238)
TRAPHANDLER_NOEC(239)
TRAPHANDLER_NOEC(240)
TRAPHANDLER_NOEC(241)
TRAPHANDLER_NOEC(242)
TRAPHANDLER_NOEC(243)
TRAPHANDLER_NOEC(244)
TRAPHANDLER_NOEC(245)
TRAPHANDLER_NOEC(246)
TRAPHANDLER_NOEC(247)
TRAPHANDLER_NOEC(248)
TRAPHANDLER_NOEC(249)
TRAPHANDLER_NOEC(250)
TRAPHANDLER_NOEC(251)
TRAPHANDLER_NOEC(252)
TRAPHANDLER_NOEC(253)
TRAPHANDLER_NOEC(254)
TRAPHANDLER_NOEC(255)

# vector table
.data
.globl vectors
vectors:
  .long vector0
  .long vector1
  .long vector2
  .long vector3
  .long vector4
  .long vector5
  .long vector6
  .long vector7
  .long vector8
  .long vector9
  .long vector10
  .long vector11
  .long vector12
  .long vector13
  .long vector14
  .long vector15
  .long vector16
  .long vector17
  .long vector18
  .long vector19
  .long vector20
  .long vector21
  .long vector22
  .long vector23
  .long vector24
  .long vector25
  .long vector26
  .long vector27
  .long vector28
  .long vector29
  .long vector30
  .long vector31
  .long vector32
  .long vector33
  .long vector34
  .long vector35
  .long vector36
  .long vector37
  .long vector38
  .long vector39
  .long vector40
  .long vector41
  .long vector42
  .long vector43
  .long vector44
  .long vector45
  .long vector46
  .long vector47
  .long vector48
  .long vector49
  .long vector50
  .long vector51
  .long vector52
  .long vector53
  .long vector54
  .long vector55
  .long vector56
  .long vector57
  .long vector58
  .long vector59
  .long vector60
  .long vector61
  .long vector62
  .long vector63
  .long vector64
  .long vector65
  .long vector66
  .long vector67
  .long vector68
  .long vector69
  .long vector70
  .long vector71
  .long vector72
  .long vector73
  .long vector74
  .long vector75
  .long vector76
  .long vector77
  .long vector78
  .long vector79
  .long vector80
  .long vector81
  .long vector82
  .long vector83
  .long vector84
  .long vector85
  .long vector86
  .long vector87
  .long vector88
  .long vector89
  .long vector90
  .long vector91
  .long vector92
  .long vector93
  .long vector94
  .long vector95
  .long vector96
  .long vector97
  .long vector98
  .long vector99
  .long vector100
  .long vector101
  .long vector102
  .long vector103
  .long vector104
  .long vector105
  .long vector106
  .long vector107
  .long vector108
  .long vector109
  .long vector110
  .long vector111
  .long vector112
  .long vector113
  .long vector114
  .long vector115
  .long vector116
  .long vector117
  .long vector118
  .long vector119
  .long vector120
  .long vector121
  .long vector122
  .long vector123
  .long vector124
  .long vector125
  .long vector126
  .long vector127
  .long vector128
  .long vector129
  .long vector130
  .long vector131
  .long vector132
  .long vector133
  .long vector134
  .long vector135
  .long vector136
  .long vector137
  .long vector138
  .long vector139
  .long vector140
  .long vector141
  .long vector142
  .long vector143
  .long vector144
  .long vector145
  .long vector146
  .long vector147
  .long vector148
  .long vector149
  .long vector150
  .long vector151
  .long vector152
  .long vector153
  .long vector154
  .long vector155
  .long vector156
  .long vector157
  .long vector158
  .long vector159
  .long vector160
  .long vector161
  .long vector162
  .long vector163
  .long vector164
  .long vector165
  .long vector166
  .long vector167
  .long vector168
  .long vector169
  .long vector170
  .long vector171
  .long vector172
  .long vector173
  .long vector174
  .long vector175
  .long vector176
  .long vector177
  .long vector178
  .long vector179
  .long vector180
  .long vector181
  .long vector182
  .long vector183
  .long vector184
  .long vector185
  .long vector186
  .long vector187
  .long vector188
  .long vector189
  .long vector190
  .long vector191
  .long vector192
  .long vector193
  .long vector194
  .long vector195
  .long vector196
  .long vector197
  .long vector198
  .long vector199
  .long vector200
  .long vector201
  .long vector202
  .long vector203
  .long vector204
  .long vector205
  .long vector206
  .long vector207
  .long vector208
  .long vector209
  .long vector210
  .long vector211
  .long vector212
  .long vector213
  .long vector214
  .long vector215
  .long vector216
  .long vector217
  .long vector218
  .long vector219
  .long vector220
  .long vector221
  .long vector222
  .long vector223
  .long vector224
  .long vector225
  .long vector226
  .long vector227
  .long vector228
  .long vector229
  .long vector230
  .long vector231
  .long vector232
  .long vector233
  .long vector234
  .long vector235
  .long vector236
  .long vector237
  .long vector238
  .long vector239
  .long vector240
  .long vector241
  .long vector242
  .long vector243
  .long vector244
  .long vector245
  .long vector246
  .long vector247
  .long vector248
  .long vector249
  .long vector250
  .long vector251
  .long vector252
  .long vector253
  .long vector254
  .long vector255


.globl sysenter_handler;
.type sysenter_handler, @function;
.align 2;
sysenter_handler:
/*
 * Lab 3: Your code here for system call handling
 */


/*
 * Lab 3: Your code here for _alltraps
 */
.globl _alltraps
_alltraps:
	# 1.push values to make the stack look like a struct Trapframe
	pushl %ds
	pushl %es
	pushal

	# 2.load GD_KD into %ds and %es
	movw $(GD_KD), %ax
	movw %ax, %ds
	movw %ax, %es

	# 3.pushl %esp to pass a pointer to the Trapframe as an argument to trap()
	pushl %esp
	call trap

