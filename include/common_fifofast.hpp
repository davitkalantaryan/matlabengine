/*
 *	File: common_fifofast.hpp
 *
 *	Created on: Jun 15, 2016
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */
#ifndef __COMMON_FIFOFAST_HPP__
#define __COMMON_FIFOFAST_HPP__

#include <malloc.h>
#include "smallmutex.h"

#ifndef LIKELY
#define LIKELY(_x_) ((_x_))
#endif

namespace common{

template <typename Type>
class FifoFast_base
{
protected:
	template <typename TypeS>struct SListStr{SListStr*   m_pNext;TypeS       m_tValue;};
public:
	FifoFast_base(int maxSize, int cashSize, SListStr<Type>**ppCashedEntries);
	virtual ~FifoFast_base();

	bool	AddElement(const Type& a_ptNew);
	bool	Extract(Type*const& a_ptBuf);
	int		size()const;

protected:
	const int			m_cnMaxSize;
	const int			m_cnCashSize;
	int					m_nIndexInCashPlus1;
	int					m_nNumOfElemets;
	mutable SmallMutex	m_Mutex;
	SListStr<Type>*		m_pFirst;
	SListStr<Type>*		m_pLast;
	SListStr<Type>**	m_ppCashedEntries;
};



template <typename Type,int CASH_SIZE_=128>
class FifoFast : public FifoFast_base<Type>
{
public:
	FifoFast(int maxSize) : FifoFast_base<Type>(maxSize, CASH_SIZE_, m_vpCashedEntries){}
	virtual ~FifoFast(){}

protected:
        SListStr<Type>* m_vpCashedEntries[CASH_SIZE_];
};


template <typename Type>
class FifoFastDyn : public FifoFast_base<Type>
{
public:
	FifoFastDyn(int maxSize, int cashSize);
	virtual ~FifoFastDyn();
};


#include "common_fifofast.tos"
} // namespace common

#endif // __COMMON_FIFOFAST_HPP__
