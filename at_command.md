# <center> AT Command </center>

## AT Command List

| **Command**     | **Description**                                                              |
| --------------- | ---------------------------------------------------------------------------- |
| **AT+FPENROLL** | Command for enrolling new fingerprint.                                       |
| **AT+FPSEARCH** | To search the whole or part of fingerprint <br> database with feature files. |
| **AT+FPREMOVE** | Remove finger print record in database.                                      |

### Error Codes

| **Name**        | **Description**                                   |
| --------------- | ------------------------------------------------- |
| **OK**          | Command execution was a success.                  |
| **ERROR**       | Unknown command or invalid number of paramenters. |
| **AUTH FAILED** | Invalid device password.                          |

<hr>

### AT+FPENROLL (Enroll new fingerprint)

Command for enrolling new fingerprint. Return the new user id.

| **Command**                 | **Response**                          |
| --------------------------- | ------------------------------------- |
| **AT+FPENROLL=\<password>** | **OK<br>[user_id] (0 - 99)** or Error |
| Response Time               | N/a                                   |

### Parameter

| \<password> | Device Security password |
| ----------- | ------------------------ |

<hr>

### AT+FPSEARCH

To search the whole or part of fingerprint database with feature files. Return the user id in fingerprint database.

| **Comman **                 | **Response**                          |
| --------------------------- | ------------------------------------- |
| **AT+FPSEARCH=\<password>** | **OK<br>[user_id] (0 - 99)** or Error |
| Response Time               | N/a                                   |

### Parameter

| \<password> | Device Security password |
| ----------- | ------------------------ |

<hr>

### AT+FPREMOVE

Remove fingerprint record in database.

| **Command**                             | **Reponse**     |
| --------------------------------------- | --------------- |
| **AT+FPREMOVE=\<password>,<[user_id]>** | **OK or Error** |
| **Response Time**                       | 500 ms          |

### Parameter

| \<password> | Device Security password  |
| ----------- | ------------------------- |
| <[user_id]> | user id range from 0 - 99 |
