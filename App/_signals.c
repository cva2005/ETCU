/*
 * _signals.c
 *
 *  Created on: 12 янв. 2016 г.
 *      Author: Перчиц А.Н.
 */
#include "_signals.h"
sig_cfg_t sig_cfg[SIG_END]; //описание сигналов
sg_t sg_st;					//состояние сигналов

//====================================================================================================================================================
void set(uint16_t sg, int32_t st)
{uint64_t result=0;

	if (sg>=SIG_END) return;

//--------------------------------------------------------------------------------
#ifdef PC
	#define p_sg sg_st.pc
	if (sig_cfg[sg].fld.deivice==PC)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//дискретные входы
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.i.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.i.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.i.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.i.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//аналоговые выходы
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				p_sg.o.a[sig_cfg[sg].fld.number]=st;
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		//конфигурация
		if (sig_cfg[sg].fld.type==CFG)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.cfg.a)/4)
				p_sg.cfg.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef BCU
#define p_sg sg_st.bcu
	if (sig_cfg[sg].fld.deivice==BCU)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//дискретные входы
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.i.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.i.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.i.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.i.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//аналоговые выходы
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				p_sg.o.a[sig_cfg[sg].fld.number]=st;
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef FC
#define p_sg sg_st.fc
	if (sig_cfg[sg].fld.deivice==FC)
		{
		//аналоговые выходы
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				p_sg.o.a[sig_cfg[sg].fld.number]=st;
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef TA
	#define p_sg sg_st.ta
	if (sig_cfg[sg].fld.deivice==TA)
		{
		//аналоговые выходы
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				p_sg.o.a[sig_cfg[sg].fld.number]=st;
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef ETCU
	#define p_sg sg_st.etcu
	if (sig_cfg[sg].fld.deivice==ETCU)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef OCU_24V
	#define p_sg sg_st.ocu
	if (sig_cfg[sg].fld.deivice==OCU_24V)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//дискретные входы
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.i.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.i.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.i.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.i.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef SSCU
	#define p_sg sg_st.sscu
	if (sig_cfg[sg].fld.deivice==SSCU)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//дискретные входы
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.i.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.i.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.i.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.i.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef HCU
	#define p_sg sg_st.hcu
	if (sig_cfg[sg].fld.deivice==HCU)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//дискретные входы
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.i.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.i.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.i.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.i.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//аналоговые выходы
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				p_sg.o.a[sig_cfg[sg].fld.number]=st;
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef PPU
	#define p_sg sg_st.ppu
	if (sig_cfg[sg].fld.deivice==PPU)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//дискретные входы
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.i.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.i.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.i.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.i.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef INCL
	#define p_sg sg_st.incl
	if (sig_cfg[sg].fld.deivice==INCL)
		{
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef PSU
	#define p_sg sg_st.psu
	if (sig_cfg[sg].fld.deivice==PSU)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef MP
	#define p_sg sg_st.mp
	if (sig_cfg[sg].fld.deivice==MP)
		{
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef CDU
	#define p_sg sg_st.cdu
	if (sig_cfg[sg].fld.deivice==CDU)
		{
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef APS
	#define p_sg sg_st.aps
	if (sig_cfg[sg].fld.deivice==APS)
		{
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif
}

//====================================================================================================================================================
int32_t st(uint16_t sg)
{
	if (sg>=SIG_END) return(0);
//--------------------------------------------------------------------------------
#ifdef PC
	#define p_sg sg_st.pc
	if (sig_cfg[sg].fld.deivice==PC)
		{
		if (sig_cfg[sg].fld.type==CFG)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.cfg)/4)
				return((int32_t)p_sg.cfg.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				return((int32_t)p_sg.o.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.d)*8)
				return((p_sg.i.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef BCU
	#define p_sg sg_st.bcu
	if (sig_cfg[sg].fld.deivice==BCU)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				return((int32_t)p_sg.o.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.d)*8)
				return((p_sg.i.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef FC
	#define p_sg sg_st.fc
	if (sig_cfg[sg].fld.deivice==FC)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				return((int32_t)p_sg.o.a[sig_cfg[sg].fld.number]);
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef TA
	#define p_sg sg_st.ta
	if (sig_cfg[sg].fld.deivice==TA)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				return((int32_t)p_sg.o.a[sig_cfg[sg].fld.number]);
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef ETCU
	#define p_sg sg_st.etcu
	if (sig_cfg[sg].fld.deivice==ETCU)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef OCU_24V
	#define p_sg sg_st.ocu
	if (sig_cfg[sg].fld.deivice==OCU_24V)
		{
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.d)*8)
				return((p_sg.i.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef SSCU
	#define p_sg sg_st.sscu
	if (sig_cfg[sg].fld.deivice==SSCU)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.d)*8)
				return((p_sg.i.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef HCU
	#define p_sg sg_st.hcu
	if (sig_cfg[sg].fld.deivice==HCU)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				return((int32_t)p_sg.o.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.d)*8)
				return((p_sg.i.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef PPU
	#define p_sg sg_st.ppu
	if (sig_cfg[sg].fld.deivice==PPU)
		{
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.d)*8)
				return((p_sg.i.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		}
	#undef p_sg
#endif
#ifdef INCL
	#define p_sg sg_st.incl
	if (sig_cfg[sg].fld.deivice==INCL)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef PSU
	#define p_sg sg_st.psu
	if (sig_cfg[sg].fld.deivice==PSU)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef MP
	#define p_sg sg_st.mp
	if (sig_cfg[sg].fld.deivice==MP)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef CDU
	#define p_sg sg_st.cdu
	if (sig_cfg[sg].fld.deivice==CDU)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
#ifdef APS
	#define p_sg sg_st.aps
	if (sig_cfg[sg].fld.deivice==APS)
		{
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		}
	#undef p_sg
#endif
//--------------------------------------------------------------------------------
return (sig_cfg[sg].fld.activ^1);
}
//====================================================================================================================================================

#ifdef _
	#define p_sg sg_st.
	if (sig_cfg[sg].fld.deivice==)
		{
		//дискретные выходы
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.o.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.o.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.o.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.o.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//дискретные входы
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number>(sizeof(p_sg.i.d)*8))
				return;
			if (st==INVERT)
				{
				p_sg.i.d^=(0x0000000000000001ULL<<((uint64_t)sig_cfg[sg].fld.number));
				}
			else
				{
				result=0x0000000000000001ULL^((uint64_t)(sig_cfg[sg].fld.activ^(st&1)));
				if (result) p_sg.i.d|=(result<<((uint64_t)sig_cfg[sg].fld.number));
				else		p_sg.i.d&=(~(1ULL<<((uint64_t)sig_cfg[sg].fld.number)));
				}
			}
		//аналоговые выходы
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				p_sg.o.a[sig_cfg[sg].fld.number]=st;
			}
		//аналоговые входы
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				p_sg.i.a[sig_cfg[sg].fld.number]=st;
			}
		//конфигурация
		if (sig_cfg[sg].fld.type==CFG)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.cfg.a)/4)
				p_sg.cfg.a[sig_cfg[sg].fld.number]=st;
			}
		}
	#undef p_sg
#endif

#ifdef _
	#define p_sg sg_st.
	if (sig_cfg[sg].fld.deivice==)
		{
		if (sig_cfg[sg].fld.type==CFG)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.cfg)/4)
				return((int32_t)p_sg.cfg.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==AI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.a)/4)
				return((int32_t)p_sg.i.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==AO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.a)/4)
				return((int32_t)p_sg.o.a[sig_cfg[sg].fld.number]);
			}
		if (sig_cfg[sg].fld.type==DI)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.i.d)*8)
				return((p_sg.i.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		if (sig_cfg[sg].fld.type==DO)
			{
			if (sig_cfg[sg].fld.number<sizeof(p_sg.o.d)*8)
				return((p_sg.o.d>>sig_cfg[sg].fld.number&1)^(sig_cfg[sg].fld.activ^1));
			}
		}
	#undef p_sg
#endif

