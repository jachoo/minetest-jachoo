/*
Minetest-c55
Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#ifndef STRFND_HEADER
#define STRFND_HEADER

#include <string>

inline std::string trim(const std::string& source, const char* delims = " \t\r\n", const int delims_cnt = 4) {

	std::string::size_type first = source.find_first_not_of(delims,0,delims_cnt);
	
	if(first==std::string::npos) return "";

	std::string::size_type last = source.find_last_not_of(delims,std::string::npos,delims_cnt);

	return source.substr(first,last-first+1);
}

class Strfnd{
    std::string tek;
    unsigned int p;
public:
    void start(std::string niinq){
        tek = niinq;
        p=0;
    }
    unsigned int where(){
        return p;
    }
    void to(unsigned int i){
        p = i;
    }
    std::string what(){
        return tek;
    }
    std::string next(std::string plop){
        //std::cout<<"tek=\""<<tek<<"\" plop=\""<<plop<<"\""<<std::endl;
        size_t n;
        std::string palautus;
        if (p < tek.size())
        {  
            //std::cout<<"\tp<tek.size()"<<std::endl;
            if ((n = tek.find(plop, p)) == std::string::npos || plop == "")
            {  
                //std::cout<<"\t\tn == string::npos || plop == \"\""<<std::endl;
                n = tek.size();
            }
            else
            {  
                //std::cout<<"\t\tn != string::npos"<<std::endl;
            }
            palautus = tek.substr(p, n-p);
            p = n + plop.length();
        }
        //else
            //std::cout<<"\tp>=tek.size()"<<std::endl;
		//std::cout<<"palautus=\""<<palautus<<"\""<<std::endl;
        return palautus;
    }
    bool atend(){
        if(p>=tek.size()) return true;
        return false;
    }
    Strfnd(std::string s){
        start(s);
    }
};

class WStrfnd{
    std::wstring tek;
    unsigned int p;
public:
    void start(std::wstring niinq){
        tek = niinq;
        p=0;
    }
    unsigned int where(){
        return p;
    }
    void to(unsigned int i){
        p = i;
    }
    std::wstring what(){
        return tek;
    }
    std::wstring next(std::wstring plop){
        //std::cout<<"tek=\""<<tek<<"\" plop=\""<<plop<<"\""<<std::endl;
        size_t n;
        std::wstring palautus;
        if (p < tek.size())
        {  
            //std::cout<<"\tp<tek.size()"<<std::endl;
            if ((n = tek.find(plop, p)) == std::wstring::npos || plop == L"")
            {  
                //std::cout<<"\t\tn == string::npos || plop == \"\""<<std::endl;
                n = tek.size();
            }
            else
            {  
                //std::cout<<"\t\tn != string::npos"<<std::endl;
            }
            palautus = tek.substr(p, n-p);
            p = n + plop.length();
        }
        //else
            //std::cout<<"\tp>=tek.size()"<<std::endl;
		//std::cout<<"palautus=\""<<palautus<<"\""<<std::endl;
        return palautus;
    }
    bool atend(){
        if(p>=tek.size()) return true;
        return false;
    }
    WStrfnd(std::wstring s){
        start(s);
    }
};

#endif

