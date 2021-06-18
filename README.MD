# fp-sisop-B04-2021 #
## Anggota Kelompok ##

NRP | NAMA
------------- | -------------
05111940000139  | Zahrotul Adillah
05111940000111  | Evelyn Sierra
05111940000059  | Dido Fabian Fayed

## Penjelasan Client ##
- Membuat socket
```c
sock = socket(AF_INET, SOCK_STREAM, 0);
```
- Jika gagal akan mengeluarkan pesan error, jika berhasil akan mengeluarkan pesan socket created
```c
        if (sock == -1) //jika gagal
        {
            printf("tidak dapat membuat socket");
        }
        puts("socket berhasil dibuat");
```
- Koneksikan ke server, jika gagal akan mengeluarkan pesa error dan return dari program
```c
        server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip address
        server.sin_family = AF_INET;
        server.sin_port = htons(7000); //port

        //koneksikan ke remote server
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("connection gagal");
            return 1;
        }
        puts("connected");
```
- Memasukkan username dan password jika bukan root(tidak menggunakan sudo)
```c
        strcpy(password, argv[4]); strcpy(username, argv[2]); 
        sprintf(unpass, "%s:::%s:::", username, password);
        send(sock, unpass, sizeof(unpass), 0);
        if (recv(sock, response, 1024, 0) < 0)
        {
            printf("login gagal\n");
            exit(1);
        }
```
- Setelah berhasil login, input command untuk dikirim ke server. 
- Jika tidak ada command dari client, maka akan mengeluarkan pesan "request gagal" dan return dari program.
- Setelah command dieksekusi oleh server, client akan mengambil respons dari server, jika tidak ada maka akan mengeluarkan pesan "respon gagal" dan break dari looping.
```c
        root = 0;
        commandTrue = 0;
        while(true){
            //kirim command ke server
            printf("%s >> ", argv[2]);
            fgets(request, sizeof(request), stdin);

            if (send(sock, request, 1024, 0) < 0) //jika tidak ada command
            {
                puts("request gagal");
                return 1;
            }
	     
	          //untuk mengambil respon dari server ke client
            if (recv(sock, response, 1024, 0) < 0)
            {
                puts("respon gagal");
                break;
            }

            puts("respon server :");
            puts(response);
            memset(&response, '\0', sizeof(response));
        }
```
