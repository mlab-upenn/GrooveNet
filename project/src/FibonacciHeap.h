/***************************************************************************
                          FibonacciHeap.h  -  Implementation of Fibonacci
						  Heap data structure (utilized pseudocode from CLR).
                             -------------------
    begin                : 2005 Jul 30
    copyright            : (C) 2005 by Daniel Weller
    email                : dweller@andrew.cmu.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FIBONACCIHEAP_H
#define _FIBONACCIHEAP_H


/**
  *@author Daniel Weller
  */

#include <stdlib.h>
#include <vector>

template<class Key, class Data> class FibonacciHeap;

template<class Key, class Data>
class FibonacciHeapNode {
public:
	FibonacciHeapNode(const Key & key, const Data & data, FibonacciHeapNode<Key, Data> * pParent = NULL, FibonacciHeapNode<Key, Data> * pLeft = NULL, FibonacciHeapNode<Key, Data> * pRight = NULL, FibonacciHeapNode<Key, Data> * pFrontChild = NULL, FibonacciHeapNode<Key, Data> * pBackChild = NULL, const size_t iDegree = 0, const bool bMarked = false);
	~FibonacciHeapNode();

	inline const Key & getKey() const {return m_Key;}

	Data m_Data;

protected:
	Key m_Key;
	FibonacciHeapNode<Key, Data> * m_pParent, * m_pLeft, * m_pRight;
	FibonacciHeapNode<Key, Data> * m_pFrontChild, * m_pBackChild;
	size_t m_iDegree; // number of children in child list
	bool m_bMarked;

	friend class FibonacciHeap<Key, Data>;

	void InsertChild(FibonacciHeapNode<Key, Data> * node);
	void RemoveChild(FibonacciHeapNode<Key, Data> * node);
	FibonacciHeapNode<Key, Data> * Duplicate(FibonacciHeapNode<Key, Data> * parent, FibonacciHeapNode<Key, Data> * left);

private:
	// disable assignment of nodes - could be messy!
	FibonacciHeapNode(const FibonacciHeapNode<Key, Data> & copy);
	FibonacciHeapNode<Key, Data> & operator = (const FibonacciHeapNode<Key, Data> & copy);
};

template<class Key, class Data>
class FibonacciHeap {
public: 
	FibonacciHeap();
	FibonacciHeap(const FibonacciHeap<Key, Data> & copy);
	~FibonacciHeap();

	FibonacciHeap<Key, Data> & operator = (const FibonacciHeap<Key, Data> & copy);

	FibonacciHeapNode<Key, Data> * InsertNode(const Key & key, const Data & data);

	inline const FibonacciHeapNode<Key, Data> * Min() const {return m_pMin;}
	inline bool IsEmpty() const {return m_nLength == 0;}

	FibonacciHeap<Key, Data> & Union(FibonacciHeap<Key, Data> & heap);
	FibonacciHeapNode<Key, Data> * ExtractMin();
	FibonacciHeapNode<Key, Data> * DecreaseKey(FibonacciHeapNode<Key, Data> * node, const Key & newKey);
	FibonacciHeapNode<Key, Data> * Delete(FibonacciHeapNode<Key, Data> * node); // equivalent to decreasing the key to below minimum, then extracting the min
	void RemoveAll();
	inline unsigned int Count() const { return m_nLength; }

protected:
	FibonacciHeapNode<Key, Data> * m_pFront, * m_pBack, * m_pMin;
	size_t m_nLength, m_iMaxDegree;
	std::vector<FibonacciHeapNode<Key, Data> *> m_vecAux; // auxilliary vector for consolidation

	void Consolidate(); // helper function for extract-min
	void Link(FibonacciHeapNode<Key, Data> * x, FibonacciHeapNode<Key, Data> * y);
	void TempInsert(FibonacciHeapNode<Key, Data> * node); // append node to end of root list (but not completely add to heap)
	void TempRemove(FibonacciHeapNode<Key, Data> * node); // remove node from root list but not entirely from heap
	void Cut(FibonacciHeapNode<Key, Data> * node, FibonacciHeapNode<Key, Data> * parent); // this and next function used in decrease-key
	void CascadingCut(FibonacciHeapNode<Key, Data> * node); // cascading effect
};

#include "FibonacciHeap.cpp"

#endif
