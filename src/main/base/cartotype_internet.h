/*
CARTOTYPE_INTERNET.H
Copyright (C) 2009 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_INTERNET_H__
#define CARTOTYPE_INTERNET_H__

#include "cartotype_base.h"
#include "cartotype_stream.h"

namespace CartoType
{

/** An object giving access to the data from a URL. */
class CInternetData
	{
	public:
	virtual ~CInternetData() { }
	virtual MInputStream& Data() = 0;

	protected:
	CInternetData() { }
	};

/**
An internet session: a connection to the internet, maintained for a period of time
and allowing multiple requests.
*/
class CInternetSession
	{
	public:
	virtual ~CInternetSession() { }
	virtual CInternetData* GetData(TResult& aError,const MString& aUrl) = 0;

	protected:
	CInternetSession() { }
	};

/** An interface class for accessing the internet. */
class MInternetAccessor
	{
	public:
	virtual CInternetSession* NewSession(TResult& aError) = 0;
	};

} // namespace CartoType

#endif // CARTOTYPE_INTERNET_H__
