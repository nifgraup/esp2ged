#define version 0.3
/*
    esp2ged #version

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
#include <stdio.h>
#include <ctime>

using namespace std;

/*  <DATA>   */
char *path; //path to espolin

char **fnofn;
int nfnofn=0;

char **snofn;
int nsnofn=0;

const char mnames[13][4] =
{
    "",
    "JAN", "FEB", "MAR",
    "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP",
    "OCT", "NOV", "DEC"
};
struct dags_type
{
    int d,m,y;
};
dags_type char2dags(const char* lina, int i)
{
    dags_type ret;
    ret.d = (unsigned char)lina[i]/8 + ((unsigned char)lina[i]%8)/8;
    ret.m = 2*((unsigned char)lina[i]%8) + (unsigned char)lina[i+1]/128;
    ret.y = (256*((unsigned char)lina[i+1]%128) + (unsigned char)lina[i+2])/8;
    return ret;
}
struct menn_type
{
    char kyn;
    int snafn1, snafn2;//#SNOFN
    int fnafn1, fnafn2;//#FNOFN
    dags_type fdag, ddag;
    int yngrasyst;//#MENN
    int por, barnifjolsk; //#POR
    int texti;//#TEXTAR
} *menn;
int nmenn=0;

struct por_type
{
    int naestasambandkarls, naestasambandkonu; //#POR
    int karl, kona, elstabarn; //#MENN
    char hjusk; //hjúskaparstaða
    dags_type hdag; //dagsetning hjúsk.
} *por;
int npor=0;

struct textar_type
{
    char* lina;
    int framhald;
} *textar;
int ntextar=0;
void printText(char* tag, ostream &file, int &texti, int &textiOffset, unsigned char merki)
{
    if((unsigned char)textar[texti].lina[textiOffset] == merki)
    {
        if(++textiOffset == 20)
        {
            texti = textar[texti].framhald;
            textiOffset = 0;
        }
        file << tag << " ";
        while((unsigned char)textar[texti].lina[textiOffset] != merki+1)
        {
            file << textar[texti].lina[textiOffset];
            if(++textiOffset == 20)
            {
                texti = textar[texti].framhald;
                textiOffset = 0;
            }
        }
        file << endl;
        if(++textiOffset == 20)
        {
            texti = textar[texti].framhald;
            textiOffset = 0;
        }
    }
}
/*  </DATA>  */


/* General file handling function */
bool file_handler(char* filename, int blocksize, void handle_init(int), void handle_function(const char*), int &datacount)
{
  char* tmpfilename = new char[strlen(path)+strlen(filename)+1];
  strcpy(tmpfilename, path);
  strcat(tmpfilename, filename);
  
  ifstream file (tmpfilename, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    int filesize = file.tellg();

    handle_init(filesize/blocksize);

    char *lina = new char[blocksize];
    for(int seek = 0; seek < filesize; seek+=blocksize)
    {
        file.seekg (seek, ios::beg);
        file.read (lina, blocksize);

        handle_function(lina);

        ++datacount;
    }
    delete lina;
//      TODO test this with valgrind
//    delete [] lina;
    file.close();
  }
  else
  {
    cout << "Could not open file " << tmpfilename << "." << endl;
    cout << endl;
    cout << "Usage: esp2ged.exe [PATH TO ESPOLIN]" << endl;
    cout << "Current path is used if no path is given." << endl;
    cout << "Output is the GEDCOM file espolin.ged." << endl;
    return false;
  }
  delete tmpfilename;
  return true;
}

void read_snofn_init(int length)
{
    snofn = new char * [length];
}
void read_snofn(const char* lina)
{
    snofn[nsnofn] = new char[strlen(lina)+1];
    strcpy(snofn[nsnofn], lina);
}

void read_fnofn_init(const int length)
{
    fnofn = new char * [length];
}
void read_fnofn(const char* lina)
{
    fnofn[nfnofn] = new char[strlen(lina)+1];
    strcpy(fnofn[nfnofn], lina);
}

void read_menn_init(int length)
{
    menn = new menn_type[length];
}
void read_menn(const char* lina)
{
    menn[nmenn].kyn = lina[0]; // 'M' eða 'F'

    menn[nmenn].snafn1 = 256*(unsigned char)lina[1]+(unsigned char)lina[2];
    menn[nmenn].snafn2 = 256*(unsigned char)lina[3]+(unsigned char)lina[4];
    menn[nmenn].fnafn1 = 256*(unsigned char)lina[5]+(unsigned char)lina[6];
    menn[nmenn].fnafn2 = 256*(unsigned char)lina[7]+(unsigned char)lina[8];
    
    menn[nmenn].fdag = char2dags(lina, 9);
    menn[nmenn].ddag = char2dags(lina, 12);

    menn[nmenn].yngrasyst = 65536*(unsigned char)lina[21]+256*(unsigned char)lina[22]+(unsigned char)lina[23];

    menn[nmenn].por = 65536*(unsigned char)lina[30]+256*(unsigned char)lina[31]+(unsigned char)lina[32];
    
    //ath. barnifjolsk er skilgreint í read_por
    menn[nmenn].barnifjolsk = 0;

    menn[nmenn].texti = 65536*(unsigned char)lina[27]+256*(unsigned char)lina[28]+(unsigned char)lina[29];
}

void read_por_init(int length)
{
    por = new por_type[length];
}
void read_por(const char* lina)
{
    por[npor].naestasambandkarls = 65536*(unsigned char)lina[3]+256*(unsigned char)lina[4]+(unsigned char)lina[5];
    por[npor].naestasambandkonu = 65536*(unsigned char)lina[6]+256*(unsigned char)lina[7]+(unsigned char)lina[8];
    por[npor].elstabarn = 65536*(unsigned char)lina[9]+256*(unsigned char)lina[10]+(unsigned char)lina[11];
    //skráir í hvaða fjölskyldum einstaklingar eru börn
    for(int naestabarn = por[npor].elstabarn; naestabarn != 0; naestabarn = menn[naestabarn].yngrasyst)
    {
        menn[naestabarn].barnifjolsk = npor;
    }
    por[npor].hjusk = lina[12];
    por[npor].hdag = char2dags(lina, 13);
    //ath. karl og kona eru skilgreind í menn loop-unni að neðan.
    por[npor].karl = 0;
    por[npor].kona = 0;
}

void read_textar_init(const int length)
{
    textar = new textar_type[length];
}
void read_textar(const char* lina)
{
    textar[ntextar].lina = new char[20+1];//TODO: optimal usage
    strncpy(textar[ntextar].lina, lina, 20);
    textar[ntextar].framhald = 65536*(unsigned char)lina[20]+256*(unsigned char)lina[21]+(unsigned char)lina[22];
}

int main(int argc, char *argv[])
{
    if(argc>1)// && argv[1] != "")
    {
        path = new char[strlen(argv[1]+15)]; // 14 = \\ + gedcom.ged + \0
        strcpy(path, argv[1]);
        path = strcat(path,"\\");
    }
    else
    {
        path = new char[2];
        path[0] = '\0';
    }
    
    //read data into memory
    //FNOFN
    cout << "Reading data from fnofn... ";
    if(!file_handler("fnofn", 24, *read_fnofn_init, *read_fnofn, nfnofn))
        return 1;
    cout << nfnofn << " surnames." << endl;
    //SNOFN
    cout << "Reading data from snofn... ";
    if(!file_handler("snofn", 28, *read_snofn_init, *read_snofn, nsnofn))
        return 1;
    cout << nsnofn << " personal names." << endl;
    //MENN
    cout << "Reading data from menn... ";
    if(!file_handler("menn", 37,  *read_menn_init,  *read_menn,  nmenn))
        return 1;
    cout << nmenn << " individuals." << endl;
    //POR
    cout << "Reading data from por... "; //depends on menn
    if(!file_handler("por", 16,   *read_por_init,   *read_por,   npor))
        return 1;
    cout << npor << " families." << endl;
    //TEXTAR
    cout << "Reading data from textar... ";
    if(!file_handler("textar", 23,   *read_textar_init,   *read_textar,   ntextar))
        return 1;
    cout << npor << " texts fractions." << endl;

    //write data into file
    strcat(path, "espolin.ged");
    ofstream file (path, ios::out|ios::binary|ios::ate);
    if (file.is_open())
    {
        file << "0 HEAD" << endl;
        file << "1 SOUR esp2ged" << endl;
        file << "2 VERS " << version << endl;
        file << "1 DEST ANY" << endl;
        time_t t = time(0); tm time = *localtime(&t);
        file << "1 DATE " << time.tm_mday << " " << mnames[time.tm_mon + 1] << " " << time.tm_year + 1900 << endl;
        file << "2 TIME " << time.tm_hour << ":" << time.tm_min << endl;
    //  file << "1 SUBM" << endl;
        file << "1 GEDC" << endl;
        file << "2 VERS 5.5" << endl;
    //file << "2 FORM LINEAGE-LINKED" << endl;
        file << "1 CHAR ISO-8859-1" << endl;
//        file << "1 CHAR CP1252" << endl;

        cout << endl << "Writing individual GEDCOM data...";
        //Skrifa út einstaklinga
        for(int i = 1; i<nmenn; i++)
        {
            int texti = menn[i].texti;
            int textiOffset = 0;
            
            file << "0 @I" << i << "@ INDI" << endl;
            file << "1 NAME " << snofn[menn[i].snafn1];
            if(menn[i].snafn2 != 0)
                file << " " << snofn[menn[i].snafn2];
            if(menn[i].fnafn1 != 0)
                file << " " << fnofn[menn[i].fnafn1];
            file <<  " /" << fnofn[menn[i].fnafn2] << "/" << endl;
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
                printText("2 PLAC", file, texti, textiOffset, 128);
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
                printText("2 PLAC", file, texti, textiOffset, 130);
            }
            if(menn[i].barnifjolsk != 0)
                file << "1 FAMC @F" << menn[i].barnifjolsk << "@" << endl;
            if(menn[i].kyn == 'M')
                for(int naestasamband = menn[i].por; naestasamband != 0; naestasamband = por[naestasamband].naestasambandkarls)
                {
                    file << "1 FAMS @F" << naestasamband << "@" << endl;
                    por[naestasamband].karl = i;
                }
            else
                for(int naestasamband = menn[i].por; naestasamband != 0; naestasamband = por[naestasamband].naestasambandkonu)
                {
                    file << "1 FAMS @F" << naestasamband << "@" << endl;
                    por[naestasamband].kona = i;
                }

            printText("1 SOUR", file, texti, textiOffset, 132);

            if((unsigned char)textar[texti].lina[textiOffset] != 0)
            {
                int charCount = 20-textiOffset;
                file << "1 NOTE " << textar[texti].lina+textiOffset;
                for(texti = textar[texti].framhald; texti != 0; texti = textar[texti].framhald)
                {
                    if(charCount > 101) // má velja aðra línulengd
                    {
                        int i = 0;
                        while(i < 20 && textar[texti].lina[i] != ' ' && (unsigned char)textar[texti].lina[i] != 0)
                        {//Smá galli í þessari lykkju: ef heilt orð nær yfir 20 stafi er orðinu skipt upp í tvennt.
                            file << textar[texti].lina[i++];
                        }
                        if((unsigned char)textar[texti].lina[i] != 0 || textar[texti].framhald != 0)
                            //þessi if-setning er til að sleppa við auka CONT í lokin
                            file << endl << "2 CONT" << textar[texti].lina+i;
                        charCount = 20-i;
                    }
                    else
                    {                        
                        file << textar[texti].lina;
                        charCount+=20;
                    }
                }
                file << endl;
            }
        }

        cout << endl << "Writing family GEDCOM data...";
        //Skrifa út fjölskyldur
        for(int i = 1; i<npor; i++)
        {
            if(por[i].kona == 0 && por[i].karl == 0 && por[i].elstabarn == 0) //getur gerst
            {
                cout << endl << "Warning: Family F" << i << " has no members, skipping.";
                continue;
            }
            file << "0 @F" << i << "@ FAM" << endl;
            if(por[i].karl != 0)
                file << "1 HUSB @I" << por[i].karl << "@" << endl;
            if(por[i].kona != 0)
                file << "1 WIFE @I" << por[i].kona << "@" << endl;
            if(por[i].hjusk==71 /*gift*/ || por[i].hjusk==88 /*skilin*/)
            {
                file << "1 MARR" << endl;
                if(por[i].hdag.y!=0)
                {
                    file << "2 DATE ";
                    if(por[i].hdag.d!=0)
                        file << por[i].hdag.d << " ";
                    if(por[i].hdag.m!=0)
                       file << mnames[por[i].hdag.m] << " ";
                    file << por[i].hdag.y << endl;
                }
            }
            if(por[i].hjusk==88) //skilin - ath. dags fylgir MARR en ekki DIV
            {
                file << "1 DIV" << endl;
            }
            for(int naestabarn = por[i].elstabarn; naestabarn != 0; naestabarn = menn[naestabarn].yngrasyst)
            {
                file << "1 CHIL @I" << naestabarn << "@" << endl;
            }
        }
        file << "0 TRLR" << endl;
        file.close();
    }
    else
    {
        cout << "Could not open the file " << path << " for writing." << endl;
        //exit(1);
        delete path;
        return 1;
    }
    cout << endl <<"The file " << path << " has been written." << endl;
    delete path;
    return 0;
}
