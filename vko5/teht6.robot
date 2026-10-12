*** Settings ***
Library   String
Library   SerialLibrary

*** Variables ***
${com}      COM3
${baud}     115200
${board}    nRF
${TERMINATOR_X}  88    # ASCII-arvo 88 on 'X'

*** Test Cases ***
Connect Serial
    Log To Console  Connecting to ${board}
    Add Port  ${com}  baudrate=${baud}  encoding=ascii
    Port Should Be Open  ${com}
    Reset Input Buffer
    Reset Output Buffer

Oikea Aika 1 Minuutti Ja 20 Sekuntia
    Testaa UART Viesti    000120X    80X

Oikea Aika Lyhyt Viive
    Testaa UART Viesti    000005X    5X

Virhe Sisaltaa Kirjaimia
    Testaa UART Viesti    00AB20X    -5X

Virhe Liian Suuret Sekuntisegmentit
    Testaa UART Viesti    001067X    -3X

Virhe Aika On Nolla
    Testaa UART Viesti    000000X    -4X

Disconnect Serial
    Log To Console  Disconnecting ${board}
    [TearDown]  Delete Port  ${com}

*** Keywords ***
Testaa UART Viesti
    [Arguments]    ${lahetettava_sarja}    ${odotettu_vastaus}
    
    Reset Input Buffer
    Reset Output Buffer
    
    Write Data   ${lahetettava_sarja}   encoding=ascii 
    Log To Console   Send sequence ${lahetettava_sarja}

    ${read} =   Read Until   terminator=${TERMINATOR_X}   encoding=ascii 
    Log To Console   Received ${read}
    
    Should Be Equal As Strings   ${read}    ${odotettu_vastaus}
    Log To Console   Tested ${read} is same as ${odotettu_vastaus}
