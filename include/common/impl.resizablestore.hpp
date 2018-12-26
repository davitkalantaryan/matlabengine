// 
// file:		impl.resizablestore.hpp
//

#ifndef __common_impl_resizablestore_hpp__
#define __common_impl_resizablestore_hpp__

#ifndef __common_resizablestore_hpp__
#error  Do not include this file directly
#include "resizablestore.hpp"
#endif

#include <stdlib.h>

template <typename Type>
common::ResizableStore<Type>::ResizableStore()
	:
	m_ptBuffer(NULL), 
	m_unSize(0),
	m_unMaxSize(0)
{
}


template <typename Type>
common::ResizableStore<Type>::~ResizableStore()
{
	for(size_t i(0);i<m_unMaxSize;++i){
		m_ptBuffer[i].~Type();
	}
	free(m_ptBuffer);
}


template <typename Type>
void common::ResizableStore<Type>::resize(size_t a_newSize)
{
	if(a_newSize>m_unMaxSize){
		Type* pcTmpBuffer = (Type*)realloc(m_ptBuffer, a_newSize * sizeof(Type));
		if(!pcTmpBuffer){return;}
		m_ptBuffer = pcTmpBuffer;
		for(;m_unMaxSize<a_newSize;++m_unMaxSize){
			new(m_ptBuffer + m_unMaxSize) Type;
		}
	}
	m_unSize = a_newSize;
}


template <typename Type>
void common::ResizableStore<Type>::resize(size_t a_newSize, void(*a_fpFunc)(Type*))
{
	if(a_newSize>m_unMaxSize){
		Type* pcTmpBuffer = (Type*)realloc(m_ptBuffer, a_newSize * sizeof(Type));
		if(!pcTmpBuffer){return;}
		m_ptBuffer = pcTmpBuffer;
		for(;m_unMaxSize<a_newSize;++m_unMaxSize){
			(*a_fpFunc)(m_ptBuffer + m_unMaxSize);
		}
	}
	m_unSize = a_newSize;
}


template <typename Type>
size_t common::ResizableStore<Type>::size()const
{
	return m_unSize;
}


template <typename Type>
Type* common::ResizableStore<Type>::buffer()
{
	return m_ptBuffer;
}


template <typename Type>
const Type*	common::ResizableStore<Type>::buffer()const
{
	return m_ptBuffer;
}


template <typename Type>
Type& common::ResizableStore<Type>::operator[](size_t a_index)
{
	return m_ptBuffer[a_index];
}


template <typename Type>
const Type& common::ResizableStore<Type>::operator[](size_t a_index)const
{
	return m_ptBuffer[a_index];
}



#endif  // #ifndef __common_impl_resizablestore_hpp__
