/*
mwer : multi-word expressions extractor
Copyright (C) 2013  Tom Bosc

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef TREE_H_
#define TREE_H_

#include "token.h"
#include "word_type.h"

namespace mwer{

template<class T> class Tree;

template<class T>
class Tree {
	private:
		T element;
		Tree<T> *father;
		std::vector<Tree<T>* > children;
	public:
		typedef typename std::vector<Tree<T>* >::const_iterator const_iterator;

		Tree(T);
		void linkWithFather(Tree<T> *father);

		int numberOfChildren();
		const_iterator childrenBegin();
		const_iterator childrenEnd();
		T getElement();
};
}

#include "tree.tpp"

#endif
