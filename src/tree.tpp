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

#include <iostream>
#include "shared.h"

namespace mwer{
template<class T>
Tree<T>::Tree(T t) :
	element(t),
	father(0),
	children()
{}



template<class T>
void Tree<T>::linkWithFather(Tree<T> *f)
{
	father = f;
	f->children.push_back(this);
}



template<class T>
typename Tree<T>::const_iterator Tree<T>::childrenBegin()
{
	return children.begin();
}



template<class T>
typename Tree<T>::const_iterator Tree<T>::childrenEnd()
{
	return children.end();
}

template<class T>
int Tree<T>::numberOfChildren()
{
	return children.size();
}



template<class T>
T Tree<T>::getElement()
{
	return element;
}
}
