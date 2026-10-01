#include "Utils/OutputSupress.h"
#include <cstdio>
#include <iostream>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <fcntl.h>
#endif

namespace
{
	void Flush()
	{
		std::cout.flush();
		std::cerr.flush();
		std::fflush(stdout);
		std::fflush(stderr);
	}

#ifdef _WIN32
	int Fileno(FILE* file) { return _fileno(file); }
	int Dup(int fd) { return _dup(fd); }
	void Dup2(int from, int to) { _dup2(from, to); }
	void Close(int fd) { _close(fd); }
	int OpenNull() { return _open("NUL", _O_WRONLY); }
#else
	int Fileno(FILE* file) { return fileno(file); }
	int Dup(int fd) { return dup(fd); }
	void Dup2(int from, int to) { dup2(from, to); }
	void Close(int fd) { close(fd); }
	int OpenNull() { return open("/dev/null", O_WRONLY); }
#endif
}

utils::OutputSuppressor::OutputSuppressor()
{
	Flush();

	this->m_oldOut = Dup(Fileno(stdout));
	this->m_oldErr = Dup(Fileno(stderr));

	const int null = OpenNull();

	Dup2(null, Fileno(stdout));
	Dup2(null, Fileno(stderr));

	Close(null);
}

utils::OutputSuppressor::~OutputSuppressor()
{
	Flush();

	Dup2(this->m_oldOut, Fileno(stdout));
	Dup2(this->m_oldErr, Fileno(stderr));

	Close(this->m_oldOut);
	Close(this->m_oldErr);
}
