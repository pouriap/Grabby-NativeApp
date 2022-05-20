#pragma once

class fatal_exception : public std::exception
{
    private:
    std::string message;

    public:
    fatal_exception(const char* msg);
	~fatal_exception(void);
    const char* what();
};

class dlg_exception : public std::exception
{
    private:
    std::string message;

    public:
    dlg_exception(const char * msg);
	~dlg_exception(void);
    const char* what();
};