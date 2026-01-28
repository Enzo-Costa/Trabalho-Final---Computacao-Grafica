## Como construir o executável?

Basta se dirigir para a pasta em que os códigos se encontram (importante que estejam todos na mesma pasta, assim como os arquivos .mtj e .mtl) e utilizar o comando

```
g++ trabalhoMain.cpp objloader.cpp -lfreeglut -lopengl32 -lglu32 -o viewer.exe
```

## Como executar o programa?

Para carregar os modelos, basta usar o comando abaixo, substituindo "nomeDoObj.obj" pelo nome do modelo que queira renderizar (no caso, buddha.obj, dragon.obj ou bunny.obj)

```
./viewer.exe nomeDoObj.obj
```
