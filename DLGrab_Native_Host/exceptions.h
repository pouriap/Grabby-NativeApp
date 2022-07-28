#pragma once

class fatal_exception : public std::exception
{
    private:
    string message;

    public:
    fatal_exception(const char* msg);
	~fatal_exception(void);
    const char* what() const throw();
};

class dlg_exception : public std::exception
{
    private:
    string message;

    public:
    dlg_exception(const char * msg);
	~dlg_exception(void);
    const char* what() const throw();
};