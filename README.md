# DECAF-compiler  

This repository has a compiler for a simplified DECAF grammar.  
This is just for the learning purpose for the people unfamiliar with compiler construction to get a know how of how to parse the code through lexemes given by the lexical analyzer.  
  
### 
This compiler has a combined Syntax analyzer with Scope checking and generating 3 Address Codes simultaneously.  
The built exe are placed in the "/exe" folder. Just run the command ./3AddrTranslation.exe test_code.txt  
It will generate following files:
1. words.txt ==> containing the lexemes
2. table.txt ==> containing all the identifiers (not used anywhere so ignore it)
3. actions.tsv ==> containing the actions taken by the syntax analyzer (i.e. the productions used on a particular lexeme and symbol)  
4. translated_code.txt ==> containing the 3 address code for the code file given.  
