#pragma once

using namespace std;

class fatal_exception : public std::exception
{
    private:
    string message;

    public:
    fatal_exception(const char* msg);
	~fatal_exception(void);
    const char* what() const throw();
};

class grb_exception : public std::exception
{
    protected:
    string message;

    public:
    grb_exception(const char * msg);
	~grb_exception(void);
    const char* what() const throw();
};

class grb_exception_gui : public grb_exception
{
public:
	grb_exception_gui(const char * msg);
	~grb_exception_gui(void);
};