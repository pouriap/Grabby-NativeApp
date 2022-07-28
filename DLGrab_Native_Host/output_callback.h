#pragma once

using namespace std;

class output_callback
{
	private:
	string dlHash;

	public:
	output_callback(const string &hash);
	~output_callback(void);
	void call(const string &output);
};

