# How to start the pintos bash

## First

```bash
docker compose up -d

docker exec -it pintos bash
```

## Second

For example,

```bash
cd /home/pintos/threads/

make

cd build
```

To test,

```bash
pintos --qemu --filesys-size=2 -- -q -f run 'echo hello'
```

```bash
pintos --qemu --filesys-size=2 -p ./tests/userprog/args-single -a args-single -- -q -f run 'args-single'

pintos --qemu --filesys-size=2 -p ./tests/userprog/write-normal -a write-normal -- -q -f run 'write-normal'

pintos --qemu --gdb --filesys-size=2 -p ./tests/userprog/write-normal -a write-normal -- -q -f run 'write-normal'
```

## GDB

Step 2
open another terminal on same machine for debugging

```bash
pintos-gdb kernel.o
```

```bash
(gdb) debugpintos
```

- Add breakpoint

  在這邊添加要在哪個 function 停下來逐行檢查

  ```bash
  (gef) break <function_name>
  ```
