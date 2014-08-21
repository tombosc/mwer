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

#include "tree.h"

#include <iostream>
#include "shared.h"

#define DEBUG_TREE 0

#if DEBUG_TREE
#define TRACE(args) std::cout<<args<<std::endl
#else
#define TRACE(args)
#endif


Tree::Tree(Token *t) :
	token(t),
	father(0),
	children()
{
	if (t == NULL) {
		std::cerr << "Error: bad tree created (nullptr token)" << std::endl;
	}
}



void Tree::linkWithFather(Tree *f)
{
	father = f;
	f->children.push_back(this);
}



bool Tree::hasChildren()
{
	return !children.empty();
}



std::vector<Tree *> &Tree::getChildren()
{
	return children;
}



WordType *Tree::getWordType()
{
	return token->getWordType();
}



int Tree::getId()
{
	return atoi(token->getFactor(ID).c_str());
}
