/*
 *  bin2obj : create linkable object from some binary files 
 *            symbol name will be the basename of filename
 *
 *  Note:     this program requires the libbfd from GNU binutils          
 *
 *                                              kxn@SMTH
 */

#include <vector>
#include <bfd.h>
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>

using namespace std;

typedef  vector <asymbol *> symlist;
typedef  vector <char>  charbuf;

string getbasename(string fn)
{
	return string(basename((char *)fn.c_str()));
}
class binfile
{
	public:
		void load(string fn)
		{
			struct stat st;
			FILE *fp;
			if (0 != stat(fn.c_str(),&st)) throw string("can not stat file " + fn);
			if (NULL == (fp = fopen(fn.c_str(),"rb"))) throw string ("can not open file" + fn);
			data.resize(st.st_size);
			fread(&data[0],st.st_size,1,fp);
			fclose(fp);
			name = getbasename(fn);
		}
		void append(string fn)
		{

			struct stat st;
			FILE *fp;
			if (0 != stat(fn.c_str(),&st)) throw string("can not stat file " + fn);
			if (NULL == (fp = fopen(fn.c_str(),"rb"))) throw string ("can not open file" + fn);
			int oldlen = data.size();
			data.resize(oldlen + st.st_size);
			fread(&data[oldlen],st.st_size,1,fp);
			fclose(fp);
		}
		const string &getname()
		{
			return name;
		}
		int length()
		{
			return data.size();
		}
		int putdata(void *buf)
		{
			memcpy(buf,&data[0],data.size());
			return data.size();
		}
		int offset;
	private:
		string name;
		charbuf data;
};

typedef map <string, int> fileindex;
typedef vector <binfile> filecollect;
class bin2objapp
{
	public:
		void addfile(string fn)
		{
			string bn = getbasename(fn);
			fileindex::iterator iter = indexes.find(bn);
			if (iter == indexes.end())
			{
				binfile n;
				int oldindex = files.size();
				indexes.insert(pair<string,int>(bn,oldindex));
				files.push_back(n);
				files[oldindex].load(fn);
			} else {
				
				files[iter->second].append(fn);
			}
		}
		void writeobj(string fn, string target)
		{
			rearrange();
			bfd *abfd =  bfd_openw(fn.c_str(), target.c_str());
			if (!abfd) throw string(bfd_errmsg(bfd_get_error()));
			bfd_set_format(abfd,bfd_object);
			asection * sect = bfd_make_section_old_way(abfd,".rodata");
			if (!sect) throw string(bfd_errmsg(bfd_get_error()));
			sect->flags = SEC_LOAD | SEC_READONLY | SEC_HAS_CONTENTS |SEC_DATA|SEC_ALLOC;
			symlist slist;
			for (filecollect::iterator iter = files.begin();
					iter != files.end() ; iter ++)
			{
				asymbol *sym = bfd_make_empty_symbol(abfd);
				sym->name = iter->getname().c_str();
				sym->section = sect;
				sym->flags = BSF_GLOBAL;
				sym->value = iter->offset;
				slist.push_back(sym);
				//printf("Added symbol %s at %d, length %d\n", sym->name, sym->value,iter->length());
			}
			if (!bfd_set_symtab(abfd,&slist[0],slist.size()))
				throw string(bfd_errmsg(bfd_get_error()));
			bfd_set_section_size(abfd,sect,data.size());
			if (!bfd_set_section_contents(abfd,sect,&data[0],0,data.size()))
				throw string(bfd_errmsg(bfd_get_error()));
			//printf("Wrote obj with %d bytes, %d symbols\n", data.size(), files.size());
			bfd_close(abfd);
		}
	private:	
		void rearrange()
		{
			int offset = 0;
			data.clear();
			for (filecollect::iterator iter = files.begin();
					iter != files.end(); iter ++)
			{
				iter->offset = offset;
				data.resize(offset + iter->length());
				iter->putdata(&data[offset]);
				offset += iter->length();
			}
		}
		fileindex indexes;
		filecollect files;
		charbuf data;
};



void helpandout(void)
{
	printf("\nbin2obj: convert binary files into a linkable object, by kxn@SMTH\n\n");
	printf("Usage: bin2obj <.o> <first .bin1> <second .bin>...\n");
	printf("       The symbol name will be the basename of each binaries.\n");
	exit(0);
}


int main(int argc,char* argv[])
{
	if (argc <3) helpandout();
	bin2objapp app;
	try {
		for (int i=2;i<argc;i++)
			app.addfile(string(argv[i]));
		app.writeobj(string(argv[1]),ARCH_TARGET);
	} catch (string e)
	{
		printf("error : %s\n", e.c_str());
	}
}

