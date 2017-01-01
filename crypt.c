#include <math.h>
#include <stdint.h>
#include <stdlib.h>
int key = 42;
void encrypt(char *password)
{
    unsigned int i;
    for(i=0;i<strlen(password);++i)
    {
        password[i] = password[i] - key;
    }
}
 
void decrypt(char *password)
{
    unsigned int i;
    for(i=0;i<strlen(password);++i)
    {
        password[i] = password[i] + key;
    }
}
/*int main()
{
    char password[20] ;
    printf("Enter the password: \n ");
    scanf("%s",password);
    printf("Passwrod     = %s\n",password);
    encrypt(password,0xFACA);
    printf("Encrypted value = %s\n",password);
    decrypt(password,0xFACA);
    printf("Decrypted value = %s\n",password);
    return 0;
}*/
