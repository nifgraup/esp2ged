/*
    esp2ged v0.1

    Copyright (C) 2008 Björgvin Ragnarsson

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along with
    this program; if not, see <http://www.gnu.org/licenses>.
*/

#include <iostream>
#include <fstream>

using namespace std;

/*  <DATA>   */
char *path; //path to espolin

char **fnofn;
int nfnofn=0;

char **snofn;
int nsnofn=0;

static const char mnames[12][12] =
{
"JAN", "FEB", "MAR",
"APR", "MAY", "JUN",
"JUL", "AUG", "SEP",
"OCT", "NOV", "DEC"
};
struct dags_type
{
    int d,m,y;
};
dags_type char2dags(char* lina, int i)
{
    dags_type ret;
    ret.d =      (unsigned char)lina[i]/8      +((unsigned char)lina[i]%8)/8;
    ret.m =    2*((unsigned char)lina[i]%8)    +(unsigned char)lina[i+1]/128;
    ret.y = (256*((unsigned char)lina[i+1]%128) +(unsigned char)lina[i+2])/8;
    return ret;
}
struct menn_type
{
    char kyn;
    int snafn1, snafn2, fnafn1, fnafn2;
    dags_type fdag, ddag;
    int yngrasyst;
    int por, barnifjolsk; //#POR
};
menn_type* menn;
int nmenn=0;

struct por_type
{
    int naestasambandkarls, naestasambandkonu; //#POR
    int karl, kona, elstabarn; //#MENN
    char hjusk; //hjúskaparstaða
    dags_type hdag; //dagsetning hjúsk.
};
por_type* por;
int npor=0;
/*  </DATA>  */


/* General file handling function */
void file_handler(char* filename, int bocksize, void handle_init(int), void handle_function(char*), int &datacount)
{
  char* tmpfilename = new char[strlen(path)];
  strcpy(tmpfilename, path);
  strcat(tmpfilename, filename);
  
  ifstream file (tmpfilename, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    ifstream::pos_type size = file.tellg();

    handle_init(size/bocksize);

    int seek = 0;
    char lina[bocksize];
    while(seek < size)
    {
        file.seekg (seek, ios::beg);
        file.read (lina, bocksize);

        handle_function(lina);

        seek+=bocksize;
        ++datacount;
    }
    file.close();
  }
  else
  {
    cout << "Gat ekki opnað skrána " << tmpfilename << "." << endl;
    cout << endl;
    cout << "Notkun: esp2ged.exe [SLÓÐ AÐ ESPÓLÍN]" << endl;
    cout << "Ef engin slóð er gefin er gert ráð fyrir að Espólín er í núverandi" << endl;
    cout << "möppu. Út skrifast GEDCOM skráin espolin.ged." << endl;
    exit(1);
  }
  delete tmpfilename;
}

void read_snofn_init(int length)
{
    snofn = new char * [length];
    for(int i = 0; i < (length); i++)
        snofn[i] = new char[28];
}
void read_snofn(char* lina)
{
    copy(lina, lina+28, snofn[nsnofn]);
}

void read_fnofn_init(int length)
{
    fnofn = new char * [length];
    for(int i = 0; i < (length); i++)
        fnofn[i] = new char[28];
}
void read_fnofn(char* lina)
{
    copy(lina, lina+24, fnofn[nfnofn]);
}

void read_menn_init(int length)
{
    menn = new menn_type[length];
}
void read_menn(char* lina)
{
    if(lina[0] == 'M')
        menn[nmenn].kyn = lina[0];
    else
        menn[nmenn].kyn = 'F';

    menn[nmenn].snafn1 = 256*(unsigned char)lina[1]+(unsigned char)lina[2];
    menn[nmenn].snafn2 = 256*(unsigned char)lina[3]+(unsigned char)lina[4];
    menn[nmenn].fnafn1 = 256*(unsigned char)lina[5]+(unsigned char)lina[6];
    menn[nmenn].fnafn2 = 256*(unsigned char)lina[7]+(unsigned char)lina[8];
    
    menn[nmenn].fdag = char2dags(lina, 9);
    menn[nmenn].ddag = char2dags(lina, 12);

    menn[nmenn].yngrasyst = 65536*(unsigned char)lina[21]+256*(unsigned char)lina[22]+(unsigned char)lina[23];

    menn[nmenn].por = 65536*(unsigned char)lina[30]+256*(unsigned char)lina[31]+(unsigned char)lina[32];
    
    //ath. ath. barnifjolsk
    menn[nmenn].barnifjolsk = 0; 
}

void read_por_init(int length)
{
    por = new por_type[length];
}
void read_por(char* lina)
{
    por[npor].naestasambandkarls = 65536*(unsigned char)lina[3]+256*(unsigned char)lina[4]+(unsigned char)lina[5];
    por[npor].naestasambandkonu = 65536*(unsigned char)lina[6]+256*(unsigned char)lina[7]+(unsigned char)lina[8];
    por[npor].elstabarn = 65536*(unsigned char)lina[9]+256*(unsigned char)lina[10]+(unsigned char)lina[11];
    por[npor].hjusk = lina[12];
    por[npor].hdag = char2dags(lina, 13);
    //ath. karl og kona eru skilgreind í menn loop-unni að neðan.
    por[npor].karl = 0;
    por[npor].kona = 0;
}

int main(int argc, char *argv[])
{
    if(argc>1)// && argv[1] != "")
    {
        path = strcat(argv[1],"\\");
    }
    else
    {
        path = new char[2];
        path[0] = '\0';
    }
    
    //FNOFN
    cout << "Les gögn úr fnofn... ";
    file_handler("fnofn", 24, read_fnofn_init, read_fnofn, nfnofn);
    //SNOFN
    cout << endl << "Les gögn úr snofn... ";
    file_handler("snofn", 28, read_snofn_init, read_snofn, nsnofn);
    //MENN
    cout << endl << "Les gögn úr menn... ";
    file_handler("menn", 37,  read_menn_init,  read_menn,  nmenn);
    //POR
    cout << endl << "Les gögn úr por... ";
    file_handler("por", 16,   read_por_init,   read_por,   npor);
    
    //skráir í hvaða fjölskyldum einstaklingar eru börn
    for(int i = 1; i<npor; i++)
    {
        int naestabarn = por[i].elstabarn;
        while(naestabarn != 0)
        {
            menn[naestabarn].barnifjolsk = i;
            naestabarn = menn[naestabarn].yngrasyst;
        }
    }    


    strcat(path, "espolin.ged");
    ofstream file (path, ios::out|ios::binary|ios::ate);
    if (file.is_open())
{
    file << "0 HEAD" << endl;
    file << "1 SOUR esp2ged 0.1" << endl;
    file << "1 DEST ANY" << endl;
//    file << "1 DATE 24 FEB 2008" << endl;
//  file << "2 TIME 1:08" << endl;
//  file << "1 SUBM" << endl;
    file << "1 GEDC" << endl;
    file << "2 VERS 5.5" << endl;
//file << "2 FORM LINEAGE-LINKED" << endl;
//    file << "1 CHAR CP850" << endl;
    file << "1 CHAR CP1252" << endl;

    cout << endl << "Skrifa út einstaklinga...";
    //Skrifa út einstaklinga
    for(int i = 1; i<nmenn; i++)
    {
        file << "0 @I" << i << "@ INDI" << endl;
        file << "1 NAME " << snofn[menn[i].snafn1] << " " << snofn[menn[i].snafn2] << " " << fnofn[menn[i].fnafn1] << " " << fnofn[menn[i].fnafn2] << endl;
        file << "1 SEX " << menn[i].kyn << endl;
        if(menn[i].fdag.y!=0)
        {
            file << "1 BIRT" << endl;
            file << "2 DATE ";
            if(menn[i].fdag.d!=0)
                file << menn[i].fdag.d << " ";
            if(menn[i].fdag.m!=0)
                file << mnames[menn[i].fdag.m] << " ";
            file << menn[i].fdag.y << endl;
        }
        if(menn[i].ddag.y!=0)
        {
            file << "1 DEAT" << endl;
            file << "2 DATE ";
            if(menn[i].ddag.d!=0)
                file << menn[i].ddag.d << " ";
            if(menn[i].ddag.m!=0)
                file << mnames[menn[i].ddag.m] << " ";
            file << menn[i].ddag.y << endl;
        }
        if(menn[i].barnifjolsk != 0)
            file << "1 FAMC @F" << menn[i].barnifjolsk << "@" << endl;
        int naestasamband = menn[i].por;
        while(naestasamband != 0)
        {
            file << "1 FAMS @F" << naestasamband << "@" << endl;
            if(menn[i].kyn == 'M')
            {
                por[naestasamband].karl = i;
                naestasamband = por[naestasamband].naestasambandkarls;
            }
            else
            {
                por[naestasamband].kona = i;
                naestasamband = por[naestasamband].naestasambandkonu;
            }
        }
    }

    cout << endl << "Skrifa út fjölskyldur...";
    //Skrifa út fjölskyldur
    for(int i = 1; i<npor; i++)
    {
        file << "0 @F" << i << "@ FAM" << endl;
        if(por[i].karl != 0)
            file << "1 HUSB @I" << por[i].karl << "@" << endl;
        if(por[i].kona != 0)
            file << "1 WIFE @I" << por[i].kona << "@" << endl;
        if(por[i].hjusk==71 || por[i].hjusk==88) //gift eða skilin
        {
            file << "1 MARR" << endl;
            if(por[i].hdag.y!=0)
            {
                file << "2 DATE ";
                if(por[i].hdag.d!=0)
                    file << por[i].hdag.d << " " << mnames[por[i].hdag.m] << " ";//TODO: assert por[i].hdag.d!=0
                file << por[i].hdag.y << endl;
            }
        }
        if(por[i].hjusk==88) //skilin
        {
            file << "1 DIV" << endl;
        }
        int naestabarn = por[i].elstabarn;
        while(naestabarn != 0)
        {
            file << "1 CHIL @I" << naestabarn << "@" << endl;
            naestabarn = menn[naestabarn].yngrasyst;
        }
    }
    file << "0 TRLR" << endl;
    file.close();
}
else
{
    cout << "Ekki tókst að opna skrána " << path << " til skriftar." << endl;
    exit(1);
}
    cout << endl <<"Skráin " << path << " hefur verið búin til." << endl;
    return 0;
}
