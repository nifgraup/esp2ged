#define version 0.5
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
#include <string.h>

using namespace std;

const char mnames[13][4] =
{
    "",
    "JAN", "FEB", "MAR",
    "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP",
    "OCT", "NOV", "DEC"
};

/*  <DATA>   */
char *path; //path to espolin

char **fnofn;
int nfnofn=0;

char **snofn;
int nsnofn=0;

struct dags_type
{
    int d,m,y;
    int certainty;
    //0 = engin óvissa, ártal líka með dagsetningu (amk. mánuði)
    //1 = engin óvissa, bara ártal
    //2 = um, +-10 ár
    //3 = (), +-50 ár
};

struct menn_type
{
    char kyn;
    int snafn1, snafn2;//#SNOFN
    int fnafn1, fnafn2;//#FNOFN
    dags_type fdag, ddag;
    int yngrasyst;//#MENN
    int por, barnifjolsk; //#POR
    int texti;//#TEXTAR
    int abud;
    int kt;
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

struct abud_type
{
    int naestaabud;
    int baer;//#BNOFN
    int uy, ucertainty; //Upphafsár
    int ly, lcertainty; //lokaár
    //0 = engin óvissa
    //8 = um, +-10 ár
    //12 = (), +-50 ár
} *abud;
int nabud=0;

struct bnofn_type
{
    char bnafn[20];
    char snafn[40];
} *bnofn;
int nbnofn = 0;
/*  </DATA>  */

/* help functions */
dags_type char2dags(const char* lina, int i)
{
    dags_type ret;
    ret.d = (unsigned char)lina[i]/8 + ((unsigned char)lina[i]%8)/8;
    ret.m = 2*((unsigned char)lina[i]%8) + (unsigned char)lina[i+1]/128;
    ret.y = (256*((unsigned char)lina[i+1]%128) + (unsigned char)lina[i+2])/8;
    ret.certainty = (unsigned char)lina[i+2]%8;
    return ret;
}

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

void printDate(ostream &file, dags_type dag)
{
    file << "2 DATE ";
    if(dag.certainty == 2) //um
        file << "BET " << dag.y-10 << " AND "<< dag.y+10;
    else if(dag.certainty == 3)//()
        file << "BET " << dag.y-50 << " AND "<< dag.y+50;
    else //mánuður og dagur er aðeins ef það er engin óvissa
    {
        if(dag.d!=0)
            file << dag.d << " ";
        if(dag.m!=0)
            file << mnames[dag.m] << " ";
            file << dag.y;
    }
    file << endl;
}

/* General file handling function */
bool file_handler(char* filename, int blocksize, void handle_init(int), void handle_function(const char*))
{
  char* tmpfilename = new char[strlen(path)+strlen(filename)+1];
  strcpy(tmpfilename, path);
  strcat(tmpfilename, filename);
  
  ifstream file (tmpfilename, ios::in|ios::binary|ios::ate);
  if (file.is_open())
  {
    int filesize = (int)file.tellg();

    handle_init(filesize/blocksize);

    char *lina = new char[blocksize];
    for(int seek = 0; seek < filesize; seek+=blocksize)
    {
        file.seekg (seek, ios::beg);
        file.read (lina, blocksize);

        handle_function(lina);
    }
    delete lina;
//      TODO test this with valgrind
//    delete [] lina;
    file.close();
  }
  else
  {
    cout << "Could not open file " << tmpfilename << ".";
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
    nsnofn++;
}

void read_fnofn_init(const int length)
{
    fnofn = new char * [length];
}
void read_fnofn(const char* lina)
{
    fnofn[nfnofn] = new char[strlen(lina)+1];
    strcpy(fnofn[nfnofn], lina);
    nfnofn++;
}

void read_menn_init(int length)
{
    menn = new menn_type[length];
}
void read_menn(const char* lina)
{
    if(lina[0] == 'K')
	   menn[nmenn].kyn = 'F'; // Kona
    else
    	menn[nmenn].kyn = lina[0]; // Karl eða óþekkt 

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
    
    menn[nmenn].abud = 65536*(unsigned char)lina[33]+256*(unsigned char)lina[34]+(unsigned char)lina[35];

    menn[nmenn].kt = (unsigned char)lina[36];
    nmenn++;
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
        menn[naestabarn].barnifjolsk = npor;
    por[npor].hjusk = lina[12];
    por[npor].hdag = char2dags(lina, 13);
    //ath. karl og kona eru skilgreind í menn loop-unni að neðan.
    por[npor].karl = 0;
    por[npor].kona = 0;
    npor++;
}

void read_textar_init(const int length)
{
    textar = new textar_type[length];
}
void read_textar(const char* lina)
{
    textar[ntextar].lina = new char[20+1];//TODO: optimize memory, ef þessi er breytt, þá verður að breyta NOTE
    strncpy(textar[ntextar].lina, lina, 20);
    textar[ntextar].framhald = 65536*(unsigned char)lina[20]+256*(unsigned char)lina[21]+(unsigned char)lina[22];
    ntextar++;
}

void read_abud_init(const int length)
{
    abud = new abud_type[length];
}
void read_abud(const char* lina)
{
    abud[nabud].naestaabud = 65536*(unsigned char)lina[3]+256*(unsigned char)lina[4]+(unsigned char)lina[5];
    abud[nabud].baer = 256*(unsigned char)lina[9]+(unsigned char)lina[10];
    abud[nabud].uy = 256*((unsigned char)lina[11]%16)+(unsigned char)lina[12];
    abud[nabud].ucertainty = (unsigned char)lina[11]/16;
    abud[nabud].ly = 256*((unsigned char)lina[13]%16)+(unsigned char)lina[14];
    abud[nabud].lcertainty = (unsigned char)lina[13]/16;
    nabud++;
}

void read_bnofn_init(const int length)
{
    bnofn = new bnofn_type[length];
}
void read_bnofn(const char* lina)
{
    strncpy(bnofn[nbnofn].bnafn, lina+3, 20);
    strncpy(bnofn[nbnofn].snafn, lina+23, 40);
    nbnofn++;
}

int returnerror()
{
    cout << endl << endl;
    cout << "Usage: esp2ged.exe [PATH TO ESPOLIN]" << endl;
    cout << "Current path is used if no path is given." << endl;
    cout << "Output is the GEDCOM file espolin.ged." << endl;
    return 1;
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

    cout << "Running esp2ged version " << version << "." << endl << endl;
    
    //read data into memory
    //FNOFN
    cout << "Reading data from fnofn... ";
    if(!file_handler("FNOFN", 24, *read_fnofn_init, *read_fnofn))
        return returnerror();
    cout << nfnofn-1 << " surnames." << endl;
    //SNOFN
    cout << "Reading data from snofn... ";
    if(!file_handler("SNOFN", 28, *read_snofn_init, *read_snofn))
        return returnerror();
    cout << nsnofn-1 << " personal names." << endl;
    //MENN
    cout << "Reading data from menn... ";
    if(!file_handler("MENN", 37,  *read_menn_init,  *read_menn))
        return returnerror();
    cout << nmenn-1 << " individuals." << endl;
    //POR
    cout << "Reading data from por... "; //depends on menn
    if(!file_handler("POR", 16,   *read_por_init,   *read_por))
        return returnerror();
    cout << npor-1 << " families." << endl;
    //TEXTAR
    cout << "Reading data from textar... ";
    if(!file_handler("TEXTAR", 23,   *read_textar_init,   *read_textar))
        return returnerror();
    cout << ntextar-1 << " text fractions." << endl;
    //ABUD
    cout << "Reading data from abud... ";
    if(!file_handler("ABUD", 15,   *read_abud_init,   *read_abud))
        cout << " Skipping." << endl;
    else //veit að þetta er Espólín 2.6
    {
        cout << nabud-1 << " resident records." << endl;
        //BNOFN
        cout << "Reading data from bnofn... ";
        if(!file_handler("BNOFN", 69,   *read_bnofn_init,   *read_bnofn))
            cout << " Skipping." << endl;
        else
            cout << nbnofn-1 << " farm names." << endl;
    }

    //write data into file
    strcat(path, "espolin.ged");
    ofstream file;
	file.open(path);
    if (file.is_open())
    {
        file << "0 HEAD" << endl;
        file << "1 SOUR esp2ged" << endl;
        file << "2 VERS " << version << endl;
        file << "1 DEST ANY" << endl;
        time_t t = time(0); tm time = *localtime(&t);
        file << "1 DATE " << time.tm_mday << " " << mnames[time.tm_mon + 1] << " " << time.tm_year + 1900 << endl;
        file << "2 TIME " << time.tm_hour << ":" << time.tm_min << endl; //TODO: add leading zero
        file << "1 SUBM @SUBM@" << endl;
        file << "1 GEDC" << endl;
        file << "2 VERS 5.5" << endl;
        file << "2 FORM LINEAGE-LINKED" << endl;
        file << "1 CHAR ANSI" << endl; //lifelines skilur ekki ANSI --> CP1252
        file << "0 @SUBM@ SUBM" << endl;
        file << "1 NAME Not Provided" << endl; //TODO: Lesa eigandann úr ES.EXE

        cout << endl << "Writing individual GEDCOM data...";
        //Skrifa út einstaklinga
        for(int i = 1; i<nmenn; i++)
        {
            if(menn[i].kyn == '!') //ógildur einstaklingur, ekki viss afhverju svona færslur verða til
            {
                cout << endl << "NOTE: Individual I" << i << " is invalid, skipping.";
                continue;
            }
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
                printDate(file, menn[i].fdag);
                printText("2 PLAC", file, texti, textiOffset, 128);
            }
            if(menn[i].ddag.y!=0)
            {
                file << "1 DEAT" << endl;
                printDate(file, menn[i].ddag);
                printText("2 PLAC", file, texti, textiOffset, 130);
            }
            if(menn[i].kt != 255) //TODO tékka hvort kt > 99
            {
                file << "1 IDNO ";
                if(menn[i].fdag.d < 10)
                    file << "0";
                file << menn[i].fdag.d;
                if(menn[i].fdag.m < 10)
                    file << "0";
                file << menn[i].fdag.m;
                if(menn[i].fdag.y%100 < 10)
                    file << "0";
                file << menn[i].fdag.y%100;
                if(menn[i].kt < 10)
                    file << "0";
                file << menn[i].kt;
                int vartala =   menn[i].fdag.d/10*3 +
                                menn[i].fdag.d%10*2 +
                                menn[i].fdag.m/10*7 +
                                menn[i].fdag.m%10*6 +
                                (menn[i].fdag.y/10)%10*5 +
                                menn[i].fdag.y%10*4 +
                                (menn[i].kt/10)%10*3 +
                                menn[i].kt%10*2;
                vartala = 11-vartala%11;
                if(vartala == 11)
                    vartala = 0;
                file << vartala << (menn[i].fdag.y/100)%10 << endl;
            }
            //Búseta
            for(int naestaabud = menn[i].abud; naestaabud != 0; naestaabud = abud[naestaabud].naestaabud)
            {//sama ártalið
                file << "1 RESI" << endl;
                file << "2 PLAC " << bnofn[abud[naestaabud].baer].bnafn << ", " << bnofn[abud[naestaabud].baer].snafn << endl;
                file << "2 DATE ";
                if(abud[naestaabud].uy == abud[naestaabud].ly && abud[naestaabud].ucertainty == abud[naestaabud].lcertainty)
                {//bara eitt ártal gefið
                    if(abud[naestaabud].ucertainty == 8) //um
                        file << "BET " << abud[naestaabud].uy-10 << " AND "<< abud[naestaabud].uy+10;
                    else if(abud[naestaabud].ucertainty == 12)//()
                        file << "BET " << abud[naestaabud].uy-50 << " AND "<< abud[naestaabud].uy+50;
                    else //engin óvissa
                        file << abud[naestaabud].uy;
                }
                else
                {//ólík ártöl
                    if(abud[naestaabud].uy != 0)
                    {
                        file << "FROM ";
                        if(abud[naestaabud].ucertainty)
                            file << "EST ";
                        file << abud[naestaabud].uy;
                    }
                    if(abud[naestaabud].ly != 0)
                    {
                        if(abud[naestaabud].uy != 0)
                            file << " ";
                        file << "TO ";
                        if(abud[naestaabud].lcertainty)
                            file << "EST ";
                        file << abud[naestaabud].ly;
                    }
                }
                file << endl;
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
                file << "1 NOTE " << textar[texti].lina+textiOffset;
                int lineCount = 0;
                for(texti = textar[texti].framhald; texti != 0; texti = textar[texti].framhald)
                {
                    if(lineCount == 3) // má velja aðra línulengd
                    {
                        if(textar[texti].framhald == 0 || textar[textar[texti].framhald].lina[0] == 0) //ekkert framhald
                            file << textar[texti].lina;
                        else
                        {
                            int i = 1;
                            while(textar[texti].lina[i] != 0 && i<20)
                                i++;
                            
                            if(textar[texti].lina[i-1] == ' ')
                            {
                                textar[texti].lina[i-1] = 0;
                                file << textar[texti].lina << endl << "2 CONT ";
                            }
                            else if(textar[textar[texti].framhald].lina[0] == ' ')
                                file << textar[texti].lina << endl << "2 CONT";
                            else
                                file << textar[texti].lina << endl << "2 CONC ";
                        }
                        lineCount = 0;
                    }
                    else
                    {
                        lineCount++;
                        file << textar[texti].lina;
                    }
                }
                file << endl;
            }
        }

        cout << endl << "Writing family GEDCOM data...";
        for(int i = 1; i<npor; i++)
        {
            if(por[i].kona == 0 && por[i].karl == 0 && por[i].elstabarn == 0) //getur gerst
            {
                cout << endl << "NOTE: Family F" << i << " has no members, skipping.";
                continue;
            }
            file << "0 @F" << i << "@ FAM" << endl;
            if(por[i].karl != 0)
                file << "1 HUSB @I" << por[i].karl << "@" << endl;
            if(por[i].kona != 0)
                file << "1 WIFE @I" << por[i].kona << "@" << endl;
//            cout << "family F" << i << ": " << (unsigned int)por[i].hjusk << endl;
            if(por[i].hjusk==0x58 /*skilin*/)
            {
                if(por[i].hdag.y!=0)
                { //ártal fylgir giftingu ef hjúskaparstaða er skráð gift/skilin
                    file << "1 MARR" << endl;
                    printDate(file, por[i].hdag);
                }
                file << "1 DIV" << endl;
            }
            else
            {
                if(por[i].hjusk==0x55)
                    file << "1 ENGA" << endl;
                else if(por[i].hjusk==0x47 /*gift*/)
                    file << "1 MARR" << endl;
                else if(por[i].hjusk==0x42)
                {
                    file << "1 EVEN" << endl;
                    file << "2 TYPE Barnsmóðir/-faðir" << endl;
                }
                else if(por[i].hjusk==0x53)
                {
                    file << "1 EVEN" << endl;
                    file << "2 TYPE Sambúð" << endl;
                }
                else if(por[i].hjusk==0x46)
                {
                    file << "1 EVEN" << endl;
                    file << "2 TYPE Sambúð slitið" << endl;
                }
                else if(por[i].hjusk==0x43)
                {
                    file << "1 EVEN" << endl;
                    file << "2 TYPE Maki(Fylgikona)" << endl;
                }
                else if(por[i].hjusk==0x44)
                {
                    file << "1 EVEN" << endl;
                    file << "2 TYPE Maki(Bústýra)" << endl;
                }
                if(por[i].hdag.y!=0)
                {
                    if(por[i].hjusk==0x20)
                    {
                        file << "1 EVEN" << endl;
                        file << "2 TYPE Óþekkt hjúskaparstaða" << endl;
                    }
                    printDate(file, por[i].hdag);
                }
            }
            for(int naestabarn = por[i].elstabarn; naestabarn != 0; naestabarn = menn[naestabarn].yngrasyst)
                file << "1 CHIL @I" << naestabarn << "@" << endl;
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
