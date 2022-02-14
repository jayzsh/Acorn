#ifndef SENSE_INTERFACE_H_
#define SENSE_INTERFACE_H_

// relative path to project root from this file
#define PATH_TO_ROOT "../"

#include <cstring>
#include <fstream>
#include <iostream>

#include "Sense.h"
#include "WN3_Path.h"
#include "Acorn_Utils.h"
#include "String_Utils.h"
#include "Synset_Pointers.h"

class Sense_Interface: public Sense
/* as an established standard for this repository, all interface class members are to be appended with 'Interface' */
{
        public:
                // from [char] synsetType
                std::string synsetTypeInterface;
                // from [std::vector<WordLexidGroup>] wordLexidGroupVector
                std::vector<std::string> synWordVectorInterface;

                // from [std::vector<Pointer>] pointers for "data.noun"
                jay_io::NounPointers /*.......*/ nounPointersInterface; 
                // from [std::vector<Pointer>] pointers for "data.verb"
                jay_io::VerbPointers /*.......*/ verbPointersInterface;
                // from [std::vector<Pointer>] pointers for "data.adj"
                jay_io::AdjectivePointers /*..*/ adjectivePointersInterface;
                // from [std::vector<Pointer>] pointers for "data.adv"
                jay_io::AdverbPointers /*.....*/ adverbPointersInterface;

                // from [std::vector<Frame>] frames for "data.verb"
                std::vector<std::string> frameInterface;

                /* 
                 *  Interface frames are rather difficult to implement and there will
                 *  be no support for this feature until there is volunteered effort.
                 *  Refer to: https://wordnet.princeton.edu/documentation/wninput5wn 
                 *  for documentation on verb frames and how to compile them.
                 */

                // constructor should call superclass constructor
                Sense_Interface(std::string, std::string);
                void previewSenseInterface();

        private:
                // from [std::string] lemma 
                std::string wordInterface;
                // from [std::string] lemma
                int wordIndexInterface;

                std::string /*................*/ parseSynsetTypeInterface /*.........*/ (char*);
                std::vector<std::string> /*...*/ parseSynWordVectorInterface /*......*/ (std::vector<WordLexidGroup>*);
                jay_io::NounPointers /*.......*/ parseNounPointersInterface /*.......*/ (std::vector<Pointer>*);
                jay_io::VerbPointers /*.......*/ parseVerbPointersInterface /*.......*/ (std::vector<Pointer>*);
                jay_io::AdjectivePointers /*..*/ parseAdjectivePointersInterface /*..*/ (std::vector<Pointer>*);
                jay_io::AdverbPointers /*.....*/ parseAdverbPointersInterface /*.....*/ (std::vector<Pointer>*);
                std::vector<std::string> /*...*/ parseFrameInterface /*..............*/ (std::vector<Frame>*);
};

Sense_Interface::Sense_Interface(std::string word, std::string sense): Sense(sense)
{
        /* class Sense_Interface constructor */
        this -> wordInterface = word;
        this -> synsetTypeInterface = parseSynsetTypeInterface(&synsetType);
        this -> synWordVectorInterface = parseSynWordVectorInterface(&wordLexidGroupVector);
        
        this -> nounPointersInterface = parseNounPointersInterface(&pointers);
        this -> verbPointersInterface = parseVerbPointersInterface(&pointers);
        this -> adjectivePointersInterface = parseAdjectivePointersInterface(&pointers);
        this -> adverbPointersInterface = parseAdverbPointersInterface(&pointers);

        // unused
        this -> frameInterface = parseFrameInterface(&frames);
}

std::string Sense_Interface::parseSynsetTypeInterface(char *synsetType)
{
        /* returns disambiguated form of char parameter synsetType */
        return parseSynsetTypeExpanded(synsetType);
}

std::vector<std::string> Sense_Interface::parseSynWordVectorInterface(std::vector<WordLexidGroup> *l_wordLexidGroupVector)
{
        /* returns manipulated form of synonym vector excluding queried word */
        std::string t;
        std::vector<std::string> l_synWordVectorInterface;

        for (int i = 0; i < (*l_wordLexidGroupVector).size(); i++) {
                t = (*l_wordLexidGroupVector)[i].word;

                if (stringToLower(&t) == wordInterface) {
                        wordIndexInterface = i + 1;
                        continue;
                }

                replaceStringUscoreWithSpace(&t);
                l_synWordVectorInterface.push_back(t);
        }
        return l_synWordVectorInterface;
}

jay_io::NounPointers Sense_Interface::parseNounPointersInterface(std::vector<Pointer> *pointers)
{
        /* parse noun pointers and create a decorated interface struct */
        std::string filepath, t;
        filepath.append(PATH_TO_ROOT);
        filepath.append(WN3DB_DAT_POS_PATH);
        std::basic_ifstream<char> datapos;
        jay_io::NounPointers p;

        for (int i = 0; i < (*pointers).size(); i++) {
                // do, if source index of the pointer refers to wordInterface (word looked up)
                if ((*pointers)[i].pos == "n" && ((*pointers)[i].source == wordIndexInterface || (*pointers)[i].source == 0)) {
                        // getlining the target sense
                        filepath.append(posFile[(*pointers)[i].pos[0]]);
                        datapos.open(filepath.c_str());
                        datapos.seekg((*pointers)[i].synsetOffset);
                        std::getline(datapos, t, '\n');

                        // parsing the getlined data row
                        Sense sense(t);

                        // push back on pointer's element with the word from the synset
                        auto pushbackOnPointerElement_With_ = [](std::vector<std::string> *element, std::string *elementOfElement) {
                                std::string w = *elementOfElement;
                                replaceStringUscoreWithSpace(&w);
                                if (!bool_ContainedIn_(&w, element)) {
                                        (*element).push_back(w);
                                }
                        };

                        // placeholder variable for the returned string
                        std::string disambiguatedPointerSymbol = parsePointerSymbol((*pointers)[i].pointerSymbol, (*pointers)[i].pos);
                        
                        // read through all synonyms in target pointer if target index is zero
                        if ((*pointers)[i].target == 0) {
                                for (int j = 0; j < sense.wordLexidGroupVector.size(); j++) {

                                        if (disambiguatedPointerSymbol == "Antonym")
                                                pushbackOnPointerElement_With_(&(p.antonym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Hypernym")
                                                pushbackOnPointerElement_With_(&(p.hypernym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Instance Hypernym")
                                                pushbackOnPointerElement_With_(&(p.instanceHypernym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Hyponym")
                                                pushbackOnPointerElement_With_(&(p.hyponym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Instance Hyponym")
                                                pushbackOnPointerElement_With_(&(p.instanceHyponym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Member holonym")
                                                pushbackOnPointerElement_With_(&(p.memberHolonym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Substance holonym")
                                                pushbackOnPointerElement_With_(&(p.substanceHolonym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Part holonym")
                                                pushbackOnPointerElement_With_(&(p.partHolonym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Member meronym")
                                                pushbackOnPointerElement_With_(&(p.memberMeronym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Substance meronym")
                                                pushbackOnPointerElement_With_(&(p.substanceMeronym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Part meronym")
                                                pushbackOnPointerElement_With_(&(p.partMeronym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Attribute")
                                                pushbackOnPointerElement_With_(&(p.attribute), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Derivationally related form")
                                                pushbackOnPointerElement_With_(&(p.derivationallyRelatedForm), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - TOPIC")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetTOPIC), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Member of this domain - TOPIC")
                                                pushbackOnPointerElement_With_(&(p.memberOfThisDomainTOPIC), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - REGION")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetREGION), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Member of this domain - REGION")
                                                pushbackOnPointerElement_With_(&(p.memberOfThisDomainTOPIC), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - USAGE")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetUSAGE), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Member of this domain - USAGE")
                                                pushbackOnPointerElement_With_(&(p.memberOfThisDomainUSAGE), &(sense.wordLexidGroupVector[j].word));
                                }
                        }

                        // read only the select synonym in target pointer if target index is non-zero
                        else {
                                for (int j = 0; j < sense.wordLexidGroupVector.size(); j++) {
                                        if (j + 1 == (*pointers)[i].target) {

                                                if (disambiguatedPointerSymbol == "Antonym")
                                                        pushbackOnPointerElement_With_(&(p.antonym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Hypernym")
                                                        pushbackOnPointerElement_With_(&(p.hypernym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Instance Hypernym")
                                                        pushbackOnPointerElement_With_(&(p.instanceHypernym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Hyponym")
                                                        pushbackOnPointerElement_With_(&(p.hyponym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Instance Hyponym")
                                                        pushbackOnPointerElement_With_(&(p.instanceHyponym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Member holonym")
                                                        pushbackOnPointerElement_With_(&(p.memberHolonym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Substance holonym")
                                                        pushbackOnPointerElement_With_(&(p.substanceHolonym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Part holonym")
                                                        pushbackOnPointerElement_With_(&(p.partHolonym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Member meronym")
                                                        pushbackOnPointerElement_With_(&(p.memberMeronym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Substance meronym")
                                                        pushbackOnPointerElement_With_(&(p.substanceMeronym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Part meronym")
                                                        pushbackOnPointerElement_With_(&(p.partMeronym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Attribute")
                                                        pushbackOnPointerElement_With_(&(p.attribute), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Derivationally related form")
                                                        pushbackOnPointerElement_With_(&(p.derivationallyRelatedForm), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - TOPIC")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetTOPIC), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Member of this domain - TOPIC")
                                                        pushbackOnPointerElement_With_(&(p.memberOfThisDomainTOPIC), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - REGION")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetREGION), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Member of this domain - REGION")
                                                        pushbackOnPointerElement_With_(&(p.memberOfThisDomainTOPIC), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - USAGE")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetUSAGE), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Member of this domain - USAGE")
                                                        pushbackOnPointerElement_With_(&(p.memberOfThisDomainUSAGE), &(sense.wordLexidGroupVector[j].word));
                                        }
                                }
                        }
                }        
        }
        
        return p;
}

jay_io::VerbPointers Sense_Interface::parseVerbPointersInterface(std::vector<Pointer> *pointers)
{
        std::string filepath, t;
        filepath.append(PATH_TO_ROOT);
        filepath.append(WN3DB_DAT_POS_PATH);
        std::basic_ifstream<char> datapos;
        jay_io::VerbPointers p;

        for (int i = 0; i < (*pointers).size(); i++) {
                // do, if source index of the pointer refers to wordInterface (word looked up)
                if ((*pointers)[i].pos == "v" && ((*pointers)[i].source == wordIndexInterface || (*pointers)[i].source == 0)) {
                        
                        // getlining the target sense
                        filepath.append(posFile[(*pointers)[i].pos[0]]);
                        datapos.open(filepath.c_str());
                        datapos.seekg((*pointers)[i].synsetOffset);
                        std::getline(datapos, t, '\n');

                        // parsing the getlined data row
                        Sense sense(t);

                        // push back on pointer's element with the word from the synset
                        auto pushbackOnPointerElement_With_ = [](std::vector<std::string> *element, std::string *elementOfElement) {
                                std::string w = *elementOfElement;
                                replaceStringUscoreWithSpace(&w);
                                if (!bool_ContainedIn_(&w, element)) {
                                        (*element).push_back(w);
                                }
                        };

                        // placeholder variable for the returned string
                        std::string disambiguatedPointerSymbol = parsePointerSymbol((*pointers)[i].pointerSymbol, (*pointers)[i].pos);
                        
                        // read through all synonyms in target pointer if target index is zero
                        if ((*pointers)[i].target == 0) {
                                for (int j = 0; j < sense.wordLexidGroupVector.size(); j++) {

                                        if (disambiguatedPointerSymbol == "Antonym")
                                                pushbackOnPointerElement_With_(&(p.antonym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Hypernym")
                                                pushbackOnPointerElement_With_(&(p.hypernym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Hyponym")
                                                pushbackOnPointerElement_With_(&(p.hyponym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Entailment")
                                                pushbackOnPointerElement_With_(&(p.entailment), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Cause")
                                                pushbackOnPointerElement_With_(&(p.cause), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Also see")
                                                pushbackOnPointerElement_With_(&(p.alsoSee), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Verb Group")
                                                pushbackOnPointerElement_With_(&(p.verbGroup), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Derivationally related form")
                                                pushbackOnPointerElement_With_(&(p.derivationallyRelatedForm), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - TOPIC")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetTOPIC), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - REGION")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetREGION), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - USAGE")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetUSAGE), &(sense.wordLexidGroupVector[j].word));
                                        }
                        }

                        // read only the select synonym in target pointer if target index is non-zero
                        else {
                                for (int j = 0; j < sense.wordLexidGroupVector.size(); j++) {
                                        if (j + 1 == (*pointers)[i].target) {

                                                if (disambiguatedPointerSymbol == "Antonym")
                                                        pushbackOnPointerElement_With_(&(p.antonym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Hypernym")
                                                        pushbackOnPointerElement_With_(&(p.hypernym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Hyponym")
                                                        pushbackOnPointerElement_With_(&(p.hyponym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Entailment")
                                                        pushbackOnPointerElement_With_(&(p.entailment), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Cause")
                                                        pushbackOnPointerElement_With_(&(p.cause), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Also see")
                                                        pushbackOnPointerElement_With_(&(p.alsoSee), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Verb Group")
                                                        pushbackOnPointerElement_With_(&(p.verbGroup), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Derivationally related form")
                                                        pushbackOnPointerElement_With_(&(p.derivationallyRelatedForm), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - TOPIC")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetTOPIC), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - REGION")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetREGION), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - USAGE")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetUSAGE), &(sense.wordLexidGroupVector[j].word));
                                        }
                                }
                        }
                }        
        }

        return p;
}

jay_io::AdjectivePointers Sense_Interface::parseAdjectivePointersInterface(std::vector<Pointer> *pointers)
{
        std::string filepath, t;
        filepath.append(PATH_TO_ROOT);
        filepath.append(WN3DB_DAT_POS_PATH);
        std::basic_ifstream<char> datapos;
        jay_io::AdjectivePointers p;

        for (int i = 0; i < (*pointers).size(); i++) {
                // do, if source index of the pointer refers to wordInterface (word looked up)
                if (((*pointers)[i].pos == "a" || (*pointers)[i].pos == "s") && ((*pointers)[i].source == wordIndexInterface || (*pointers)[i].source == 0)) {
                        
                        // getlining the target sense
                        filepath.append(posFile[(*pointers)[i].pos[0]]);
                        datapos.open(filepath.c_str());
                        datapos.seekg((*pointers)[i].synsetOffset);
                        std::getline(datapos, t, '\n');

                        // parsing the getlined data row
                        Sense sense(t);

                        // push back on pointer's element with the word from the synset
                        auto pushbackOnPointerElement_With_ = [](std::vector<std::string> *element, std::string *elementOfElement) {
                                std::string w = *elementOfElement;
                                replaceStringUscoreWithSpace(&w);
                                if (!bool_ContainedIn_(&w, element)) {
                                        (*element).push_back(w);
                                }
                        };

                        // placeholder variable for the returned string
                        std::string disambiguatedPointerSymbol = parsePointerSymbol((*pointers)[i].pointerSymbol, (*pointers)[i].pos);
                        
                        // read through all synonyms in target pointer if target index is zero
                        if ((*pointers)[i].target == 0) {
                                for (int j = 0; j < sense.wordLexidGroupVector.size(); j++) {

                                        if (disambiguatedPointerSymbol == "Antonym")
                                                pushbackOnPointerElement_With_(&(p.antonym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Similar to")
                                                pushbackOnPointerElement_With_(&(p.similarTo), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Participle of verb")
                                                pushbackOnPointerElement_With_(&(p.participleOfVerb), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Pertainym (pertains to noun)")
                                                pushbackOnPointerElement_With_(&(p.pertainym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Attribute")
                                                pushbackOnPointerElement_With_(&(p.attribute), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Also see")
                                                pushbackOnPointerElement_With_(&(p.alsoSee), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - TOPIC")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetTOPIC), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - REGION")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetREGION), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - USAGE")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetUSAGE), &(sense.wordLexidGroupVector[j].word));
                                }
                        }

                        // read only the select synonym in target pointer if target index is non-zero
                        else {
                                for (int j = 0; j < sense.wordLexidGroupVector.size(); j++) {
                                        if (j + 1 == (*pointers)[i].target) {

                                                if (disambiguatedPointerSymbol == "Antonym")
                                                        pushbackOnPointerElement_With_(&(p.antonym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Similar to")
                                                        pushbackOnPointerElement_With_(&(p.similarTo), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Participle of verb")
                                                        pushbackOnPointerElement_With_(&(p.participleOfVerb), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Pertainym (pertains to noun)")
                                                        pushbackOnPointerElement_With_(&(p.pertainym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Attribute")
                                                        pushbackOnPointerElement_With_(&(p.attribute), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Also see")
                                                        pushbackOnPointerElement_With_(&(p.alsoSee), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - TOPIC")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetTOPIC), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - REGION")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetREGION), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - USAGE")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetUSAGE), &(sense.wordLexidGroupVector[j].word));
                                        }
                                }
                        }
                }        
        }

        return p;
}

jay_io::AdverbPointers Sense_Interface::parseAdverbPointersInterface(std::vector<Pointer> *pointers)
{
        std::string filepath, t;
        filepath.append(PATH_TO_ROOT);
        filepath.append(WN3DB_DAT_POS_PATH);
        std::basic_ifstream<char> datapos;
        jay_io::AdverbPointers p;

        for (int i = 0; i < (*pointers).size(); i++) {
                // do, if source index of the pointer refers to wordInterface (word looked up)
                if ((*pointers)[i].pos == "r" && ((*pointers)[i].source == wordIndexInterface || (*pointers)[i].source == 0)) {
                        
                        // getlining the target sense
                        filepath.append(posFile[(*pointers)[i].pos[0]]);
                        datapos.open(filepath.c_str());
                        datapos.seekg((*pointers)[i].synsetOffset);
                        std::getline(datapos, t, '\n');

                        // parsing the getlined data row
                        Sense sense(t);

                        // push back on pointer's element with the word from the synset
                        auto pushbackOnPointerElement_With_ = [](std::vector<std::string> *element, std::string *elementOfElement) {
                                std::string w = *elementOfElement;
                                replaceStringUscoreWithSpace(&w);
                                if (!bool_ContainedIn_(&w, element)) {
                                        (*element).push_back(w);
                                }
                        };

                        // placeholder variable for the returned string
                        std::string disambiguatedPointerSymbol = parsePointerSymbol((*pointers)[i].pointerSymbol, (*pointers)[i].pos);
                        
                        // read through all synonyms in target pointer if target index is zero
                        if ((*pointers)[i].target == 0) {
                                for (int j = 0; j < sense.wordLexidGroupVector.size(); j++) {

                                        if (disambiguatedPointerSymbol == "Antonym")
                                                pushbackOnPointerElement_With_(&(p.antonym), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Derived from adjective")
                                                pushbackOnPointerElement_With_(&(p.derivedFromAdjective), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - TOPIC")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetTOPIC), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - REGION")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetREGION), &(sense.wordLexidGroupVector[j].word));

                                        else if (disambiguatedPointerSymbol == "Domain of synset - USAGE")
                                                pushbackOnPointerElement_With_(&(p.domainOfSynsetUSAGE), &(sense.wordLexidGroupVector[j].word));
                                }
                        }

                        // read only the select synonym in target pointer if target index is non-zero
                        else {
                                for (int j = 0; j < sense.wordLexidGroupVector.size(); j++) {
                                        if (j + 1 == (*pointers)[i].target) {

                                                if (disambiguatedPointerSymbol == "Antonym")
                                                        pushbackOnPointerElement_With_(&(p.antonym), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Derived from adjective")
                                                        pushbackOnPointerElement_With_(&(p.derivedFromAdjective), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - TOPIC")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetTOPIC), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - REGION")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetREGION), &(sense.wordLexidGroupVector[j].word));

                                                else if (disambiguatedPointerSymbol == "Domain of synset - USAGE")
                                                        pushbackOnPointerElement_With_(&(p.domainOfSynsetUSAGE), &(sense.wordLexidGroupVector[j].word));
                                        }
                                }
                        }
                }        
        }

        return p;
}

std::vector<std::string> Sense_Interface::parseFrameInterface(std::vector<Frame>* l_frames)
{
        return {""};
}

void Sense_Interface::previewSenseInterface()
{
        std::string out, t, tab = "\t", colonSpace = ": ", newline = "\n";

        auto prettyPrintVectorOfStrings = [&] (std::vector<std::string> *vos) -> std::string {
                std::string a;
                for (int i = 0; i < (*vos).size(); i++)
                {
                        if ((*vos).size() - 1 != i) {
                                t = (*vos)[i];
                                t.append(", ");
                                a.append(t);
                        } else
                                a.append((*vos)[i]);
                }
                return a;
        };

        auto printNounPointers = [&] () -> std::string {
                out.clear();
                if (!nounPointersInterface.antonym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 0] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.antonym) + newline);
                if (!nounPointersInterface.hypernym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 1] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.hypernym) + newline);
                if (!nounPointersInterface.instanceHypernym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 2] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.instanceHypernym) + newline);
                if (!nounPointersInterface.hyponym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 3] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.hyponym) + newline);
                if (!nounPointersInterface.instanceHyponym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 4] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.instanceHyponym) + newline);
                if (!nounPointersInterface.memberHolonym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 5] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.memberHolonym) + newline);
                if (!nounPointersInterface.substanceHolonym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 6] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.substanceHolonym) + newline);
                if (!nounPointersInterface.partHolonym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 7] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.partHolonym) + newline);
                if (!nounPointersInterface.memberMeronym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 8] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.memberMeronym) + newline);
                if (!nounPointersInterface.substanceMeronym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[ 9] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.substanceMeronym) + newline);
                if (!nounPointersInterface.partMeronym.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[10] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.partMeronym) + newline);
                if (!nounPointersInterface.attribute.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[11] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.attribute) + newline);
                if (!nounPointersInterface.derivationallyRelatedForm.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[12] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.derivationallyRelatedForm) + newline);
                if (!nounPointersInterface.domainOfSynsetTOPIC.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[13] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.domainOfSynsetTOPIC) + newline);
                if (!nounPointersInterface.memberOfThisDomainTOPIC.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[14] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.memberOfThisDomainTOPIC) + newline);
                if (!nounPointersInterface.domainOfSynsetREGION.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[15] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.domainOfSynsetREGION) + newline);
                if (!nounPointersInterface.memberOfThisDomainREGION.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[16] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.memberOfThisDomainREGION) + newline);
                if (!nounPointersInterface.domainOfSynsetUSAGE.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[17] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.domainOfSynsetUSAGE) + newline);
                if (!nounPointersInterface.memberOfThisDomainUSAGE.empty())
                        out.append(tab + nounPointerElemDisambiguationIndex[18] + colonSpace + prettyPrintVectorOfStrings(&nounPointersInterface.memberOfThisDomainUSAGE) + newline);
                return out;
        };

        auto printVerbPointers = [&] () -> std::string {
                out.clear();
                if (!verbPointersInterface.antonym.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 0] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.antonym) + newline);
                if (!verbPointersInterface.hypernym.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 1] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.hypernym) + newline);
                if (!verbPointersInterface.hyponym.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 2] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.hyponym) + newline);
                if (!verbPointersInterface.entailment.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 3] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.entailment) + newline);
                if (!verbPointersInterface.cause.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 4] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.cause) + newline);
                if (!verbPointersInterface.alsoSee.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 5] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.alsoSee) + newline);
                if (!verbPointersInterface.verbGroup.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 6] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.verbGroup) + newline);
                if (!verbPointersInterface.derivationallyRelatedForm.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 7] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.derivationallyRelatedForm) + newline);
                if (!verbPointersInterface.domainOfSynsetTOPIC.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 8] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.domainOfSynsetTOPIC) + newline);
                if (!verbPointersInterface.domainOfSynsetREGION.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[ 9] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.domainOfSynsetREGION) + newline);
                if (!verbPointersInterface.domainOfSynsetUSAGE.empty())
                        out.append(tab + verbPointerElemDisambiguationIndex[10] + colonSpace + prettyPrintVectorOfStrings(&verbPointersInterface.domainOfSynsetUSAGE) + newline);
                return out;
        };

        auto printAdjectivePointers = [&] () -> std::string {
                out.clear();
                if (!adjectivePointersInterface.antonym.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 0] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.antonym) + newline);
                if (!adjectivePointersInterface.similarTo.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 1] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.similarTo) + newline);
                if (!adjectivePointersInterface.participleOfVerb.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 2] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.participleOfVerb) + newline);
                if (!adjectivePointersInterface.pertainym.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 3] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.pertainym) + newline);
                if (!adjectivePointersInterface.attribute.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 4] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.attribute) + newline);
                if (!adjectivePointersInterface.alsoSee.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 5] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.alsoSee) + newline);
                if (!adjectivePointersInterface.domainOfSynsetTOPIC.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 6] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.domainOfSynsetTOPIC) + newline);
                if (!adjectivePointersInterface.domainOfSynsetREGION.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 7] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.domainOfSynsetREGION) + newline);
                if (!adjectivePointersInterface.domainOfSynsetUSAGE.empty())
                        out.append(tab + adjectivePointerElemDisambiguationIndex[ 8] + colonSpace + prettyPrintVectorOfStrings(&adjectivePointersInterface.domainOfSynsetUSAGE) + newline);
                return out;
        };

        auto printAdverbPointers = [&] () -> std::string {
                out.clear();
                if (!adverbPointersInterface.antonym.empty())
                        out.append(tab + adverbPointerElemDisambiguationIndex[ 0] + colonSpace + prettyPrintVectorOfStrings(&adverbPointersInterface.antonym) + newline);
                if (!adverbPointersInterface.derivedFromAdjective.empty())
                        out.append(tab + adverbPointerElemDisambiguationIndex[ 1] + colonSpace + prettyPrintVectorOfStrings(&adverbPointersInterface.derivedFromAdjective) + newline);
                if (!adverbPointersInterface.domainOfSynsetTOPIC.empty())
                        out.append(tab + adverbPointerElemDisambiguationIndex[ 2] + colonSpace + prettyPrintVectorOfStrings(&adverbPointersInterface.domainOfSynsetTOPIC) + newline);
                if (!adverbPointersInterface.domainOfSynsetREGION.empty())
                        out.append(tab + adverbPointerElemDisambiguationIndex[ 3] + colonSpace + prettyPrintVectorOfStrings(&adverbPointersInterface.domainOfSynsetREGION) + newline);
                if (!adverbPointersInterface.domainOfSynsetUSAGE.empty())
                        out.append(tab + adverbPointerElemDisambiguationIndex[ 4] + colonSpace + prettyPrintVectorOfStrings(&adverbPointersInterface.domainOfSynsetUSAGE) + newline);
                return out;
        };

        std::cout << "Part of speech:     " << synsetTypeInterface << std::endl;
        std::cout << "Synonym set:        " << prettyPrintVectorOfStrings(&synWordVectorInterface) << std::endl;
        std::cout << "Noun pointers:      " << newline << printNounPointers() << std::endl;
        std::cout << "Verb pointers:      " << newline << printVerbPointers() << std::endl;
        std::cout << "Adjective pointers: " << newline << printAdjectivePointers() << std::endl;
        std::cout << "Adverb pointers:    " << newline << printAdverbPointers() << std::endl;
        std::cout << "wII:                " << wordIndexInterface << newline;
}

#endif /* SENSE_INTERFACE_H_ */