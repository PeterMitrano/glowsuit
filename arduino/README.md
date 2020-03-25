# Suits

This folder has the code that is uploaded to each suit on it. This code contains the choreography as well as XBee code to ensure the suits stay in time and can resume choreo if they're shut down accidentally. The suits also listen for the following commands via XBee:

| name | data | description |
|------|------|-------------|
| start| ?    | start the choreo |
| stop | ?    | stop the choreo, turn off all the lights |
| all  | ?    | turn on all the lights. stops choreo if it's running. |

