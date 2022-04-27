Tohle byl pokus o implementaci solveru na Tykadla pomoci "helperu", objektu,
ktere udrzuji o polickach s cislem podobne info, jako resitel, a delaji uvahy 
podobnym zpusobem. Skoncilo to uspechem pouze castecnym; z ne zcela pochopenych
duvodu dochazi k tomu, ze se mi jako reseni vraci i takove gridy, kde nektera
tykadla jsou prerusovana, ackoli PropagatorRoot tuto vlastnost osetruje. Zda se,
ze pricinou je, ze vinou assignovani vsech promennych se nektere propagate() metody
nezavolaji, ackoli jejich propagatory maji PC_INT_VAL. 

Je to jenom povest a je to divne. Nedava mi to smysl a napadl me vyrazne jednodussi
postup, jak Tykadla implementovat, takze zde koncim.
