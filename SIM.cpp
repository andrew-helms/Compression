//On my honor, I have neither given nor received any unauthorized aid on this assignment

#include <vector>
#include <iostream>
#include <fstream>
#include "stdio.h"
#include <string.h>
#include <string>
#include <sstream>
#include <math.h>
#include <bitset>

struct DictEntry
{
public:
    unsigned int value;
    unsigned int count;
    
    DictEntry(unsigned int val, unsigned int cnt) : value(val), count(cnt) {}
};

void Compress();
void Decompress();

int main(int argc, char *argv[])
{
    if (std::stoi(argv[1]) == 1)
        Compress();
    else if (std::stoi(argv[1]) == 2)
        Decompress();
    
    return 0;
}

void Compress()
{
    /* INITIALIZE DICTIONARY FROM FILE AND SAVE PROGRAM*/
    
    std::ifstream file;
    
    std::vector<unsigned int> program;
    std::vector<DictEntry> dictionary;
    
    file.open("original.txt");
    
    std::string line;
    while (std::getline(file, line))
    {
        unsigned int iLine = 0;
        
        for (int i = 0; i < 32; i++)
            if (line[i] == '1')
                iLine += pow(2,31-i);
            
        program.push_back(iLine);
        
        bool inDict = false;
        for (std::vector<DictEntry>::iterator iter = dictionary.begin(); iter != dictionary.end(); iter++)
        {
            if (iter->value == iLine)
            {
                iter->count++;
                inDict = true;
                break;
            }
        }
        
        if (!inDict)
            dictionary.push_back(DictEntry(iLine, 1));
    }
    
    file.close();
    
    /* SORT DICTIONARY AND FILTER TO SIZE */
    
    for (int i = 0; i < 16; i++)
    {
        bool sorted = true;
        
        for (int j = dictionary.size() - 1; j > i; j--)
        {
            if (dictionary[j].count > dictionary[j-1].count)
            {
                DictEntry temp = dictionary[j];
                dictionary[j] = dictionary[j-1];
                dictionary[j-1] = temp;
                
                sorted = false;
            }
        }
        
        if (sorted)
            break;
    }
    
    while (dictionary.size() > 16)
        dictionary.pop_back();
    
    /* COMPRESS LINES AND STORE AS STRINGS */
    
    std::string compProg = "";
    int RLEctr = 0;
    bool RLEof = false; //RLE overflow
    
    for (int i = 0; i < program.size(); i++)
    {
        if (i > 0 && program[i-1] == program[i] && !RLEof)
        {
            RLEctr++;
            
            if (RLEctr == 8)
            {
                compProg += "001" + std::bitset<3>(RLEctr-1).to_string();
                RLEctr = 0;
                RLEof = true;
            }
            
            continue;
        }
        else if (RLEctr > 0)
        {
            compProg += "001" + std::bitset<3>(RLEctr-1).to_string();
            RLEctr = 0;
        }
        
        if (RLEof)
            RLEof = false;
        
        for (int j = 0; j < dictionary.size(); j++) //loop for direct match
        {
            if (dictionary[j].value == program[i])
            {
                compProg += "111" + std::bitset<4>(j).to_string();
                goto ENDLOOP;
            }
        }
        
        for (int j = 0; j < dictionary.size(); j++) //loop for 1 bit mismatch
        {
            int diff = dictionary[j].value ^ program[i];
            int index;
            
            for (index = 0; diff % 2 == 0 && diff > 0; index++)
                diff = diff>>1;
            
            if (diff == 1)
            {
                compProg += "011" + std::bitset<5>(31-index).to_string() + std::bitset<4>(j).to_string();
                goto ENDLOOP;
            }
        }
        
        for (int j = 0; j < dictionary.size(); j++) //loop for 2 bit consecutive mismatch
        {
            int diff = dictionary[j].value ^ program[i];
            int index;
            
            for (index = 0; diff % 2 == 0 && diff > 0; index++)
                diff = diff>>1;
            
            if (diff == 2)
            {
                compProg += "100" + std::bitset<5>(30-index).to_string() + std::bitset<4>(j).to_string();
                goto ENDLOOP;
            }
        }
        
        for (int j = 0; j < dictionary.size(); j++) //loop for 4 bit consecutive mismatch
        {
            int diff = dictionary[j].value ^ program[i];
            int index;
            
            for (index = 0; diff % 2 == 0 && diff > 0; index++)
                diff = diff>>1;
            
            if (diff == 4)
            {
                compProg += "101" + std::bitset<5>(28-index).to_string() + std::bitset<4>(j).to_string();
                goto ENDLOOP;
            }
        }
        
        for (int j = 0; j < dictionary.size(); j++) //loop for bitmask;
        {
            unsigned int diff = dictionary[j].value ^ program[i];
            unsigned int oldDiff = diff;
            int index;
            
            for (index = 0; diff % 2 == 0 && diff > 0; index++)
                diff = diff>>1;
            
            if (diff < 16)
            {
                index = 28-index;
                
                if (index < 0)
                {
                    diff = diff<<(-1*index);
                    index = 0;
                }
                
                compProg += "010" + std::bitset<5>(index).to_string() + std::bitset<4>(diff).to_string() + std::bitset<4>(j).to_string();
                goto ENDLOOP;
            }
        }
        
        for (int j = 0; j < dictionary.size(); j++) //loop for 2 bit non-consecutive mismatch
        {
            int diff = dictionary[j].value ^ program[i];
            int index1;
            int index2;
            
            for (index1 = 0; diff % 2 == 0 && diff > 0; index1++)
                diff = diff>>1;
                
            diff = diff>>1;
                
            for (index2 = index1 + 1; diff % 2 == 0 && diff > 0; index2++)
                diff = diff>>1;
            
            if (diff == 1)
            {
                compProg += "110" + std::bitset<5>(31-index2).to_string() + std::bitset<5>(31-index1).to_string() + std::bitset<4>(j).to_string();
                goto ENDLOOP;
            }
        }
        
        compProg += "000" + std::bitset<32>(program[i]).to_string();
        
        ENDLOOP:;
    }
    
    /* FORMAT AND PRINT COMPRESSED PROGRAM TO FILE */
    
    while (compProg.size() % 32 != 0)
        compProg += '0';
    
    for (int i = 32; i < compProg.size(); i += 33)
        compProg.insert(i, 1, '\n');
    
    compProg += "\nxxxx";
    
    for (DictEntry entry : dictionary)
        compProg += '\n' + std::bitset<32>(entry.value).to_string();
    
    std::ofstream outputFile;
    outputFile.open("cout.txt");
    
    outputFile << compProg;
    
    outputFile.close();
}

void Decompress()
{    
    /* INITIALIZE DICTIONARY FROM FILE AND SAVE PROGRAM*/
    
    std::ifstream file;
    
    std::string compProg = "";
    std::vector<unsigned int> dictionary;
    
    file.open("compressed.txt");
    
    bool saveDict = false;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.compare("xxxx") == 0)
        {
            saveDict = true;
            continue;
        }
        
        if (saveDict)
        {
            unsigned int iLine = 0;
        
            for (int i = 0; i < 32; i++)
                if (line[i] == '1')
                    iLine += pow(2,31-i);
                
            dictionary.push_back(iLine);
        }
        else
        {
            compProg += line;
        }
    }
    
    /* DECOMPRESS PROGRAM BY LINE */
    
    int index = 0;
    std::vector<unsigned int> program;
    
    while (index < compProg.size())
    {
        std::string code = compProg.substr(index, 3);
        index += 3;
        
        unsigned int iCode = 0;
        
        for (int i = 0; i < 3; i++)
            if (code[i] == '1')
                iCode += pow(2,2-i);
        
        switch (iCode)
        {
            case 0:
            {
                std::string progLine = compProg.substr(index, 32);
                index += 32;
                
                if (progLine.size() < 32)
                    break;
                
                unsigned int iLine = 0;
                
                for (int i = 0; i < 32; i++)
                    if (progLine[i] == '1')
                        iLine += pow(2,31-i);
                
                program.push_back(iLine);
                
                break;
            }
            case 1:
            {
                std::string RLE = compProg.substr(index, 3);
                index += 3;
                
                unsigned int iRLE = 1;
                
                for (int i = 0; i < 3; i++)
                    if (RLE[i] == '1')
                        iRLE += pow(2,2-i);
                
                for (int i = 0; i < iRLE; i++)
                    program.push_back(program.back());
                
                break;
            }
            case 2:
            {
                std::string parsing = compProg.substr(index, 5);
                index += 5;
                
                int startLoc = 0;
                
                for (int i = 0; i < 5; i++)
                    if (parsing[i] == '1')
                        startLoc += pow(2,4-i);
                    
                startLoc = 28 - startLoc;
                
                parsing = compProg.substr(index, 4);
                index += 4;
                
                unsigned int mask = 0;
                
                for (int i = 0; i < 4; i++)
                    if (parsing[i] == '1')
                        mask += pow(2,3-i);
                    
                parsing = compProg.substr(index, 4);
                index += 4;
                
                int dictIndex = 0;
                
                for (int i = 0; i < 4; i++)
                    if (parsing[i] == '1')
                        dictIndex += pow(2,3-i);
                
                mask = mask<<startLoc;
                
                program.push_back(dictionary[dictIndex]^mask);
                
                break;
            }
            case 3:
            {
                std::string parsing = compProg.substr(index, 5);
                index += 5;
                
                int startLoc = 0;
                
                for (int i = 0; i < 5; i++)
                    if (parsing[i] == '1')
                        startLoc += pow(2,4-i);
                    
                startLoc = 31 - startLoc;
                
                parsing = compProg.substr(index, 4);
                index += 4;
                
                int dictIndex = 0;
                
                for (int i = 0; i < 4; i++)
                    if (parsing[i] == '1')
                        dictIndex += pow(2,3-i);
                    
                int mask = 1<<startLoc;
                
                program.push_back(dictionary[dictIndex]^mask);
                
                break;
            }
            case 4:
            {
                std::string parsing = compProg.substr(index, 5);
                index += 5;
                
                int startLoc = 0;
                
                for (int i = 0; i < 5; i++)
                    if (parsing[i] == '1')
                        startLoc += pow(2,4-i);
                    
                startLoc = 30 - startLoc;
                
                parsing = compProg.substr(index, 4);
                index += 4;
                
                int dictIndex = 0;
                
                for (int i = 0; i < 4; i++)
                    if (parsing[i] == '1')
                        dictIndex += pow(2,3-i);
                    
                int mask = 3<<startLoc;
                
                program.push_back(dictionary[dictIndex]^mask);
                
                break;
            }
            case 5:
            {
                std::string parsing = compProg.substr(index, 5);
                index += 5;
                
                int startLoc = 0;
                
                for (int i = 0; i < 5; i++)
                    if (parsing[i] == '1')
                        startLoc += pow(2,4-i);
                    
                startLoc = 28 - startLoc;
                
                parsing = compProg.substr(index, 4);
                index += 4;
                
                int dictIndex = 0;
                
                for (int i = 0; i < 4; i++)
                    if (parsing[i] == '1')
                        dictIndex += pow(2,3-i);
                    
                int mask = 15<<startLoc;
                
                program.push_back(dictionary[dictIndex]^mask);
                
                break;
            }
            case 6:
            {
                std::string parsing = compProg.substr(index, 5);
                index += 5;
                
                int startLoc1 = 0;
                
                for (int i = 0; i < 5; i++)
                    if (parsing[i] == '1')
                        startLoc1 += pow(2,4-i);
                    
                startLoc1 = 31 - startLoc1;
                
                parsing = compProg.substr(index, 5);
                index += 5;
                
                int startLoc2 = 0;
                
                for (int i = 0; i < 5; i++)
                    if (parsing[i] == '1')
                        startLoc2 += pow(2,4-i);
                    
                startLoc2 = 31 - startLoc2;
                
                parsing = compProg.substr(index, 4);
                index += 4;
                
                int dictIndex = 0;
                
                for (int i = 0; i < 4; i++)
                    if (parsing[i] == '1')
                        dictIndex += pow(2,3-i);
                    
                int mask1 = 1<<startLoc1;
                int mask2 = 1<<startLoc2;
                
                program.push_back(dictionary[dictIndex]^(mask1|mask2));
                
                break;
            }
            case 7:
            {
                std::string parsing = compProg.substr(index, 4);
                index += 4;
                
                int dictIndex = 0;
                
                for (int i = 0; i < 4; i++)
                    if (parsing[i] == '1')
                        dictIndex += pow(2,3-i);
                    
                program.push_back(dictionary[dictIndex]);
                
                break;
            }
        }
    }
    
    std::ofstream outputFile;
    outputFile.open("dout.txt");
    
    for (unsigned int line : program)
        outputFile << std::bitset<32>(line) << std::endl;
    
    outputFile.close();
}