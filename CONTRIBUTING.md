# Contributing to AstraRTOS

## Commit Messages

Write commit messages with a title and description:

```
<area>: <short summary>

<detailed description of what and why>

Signed-off-by: Your Name <your@email.com>
```

### Example

```
scheduler: add task switching

Implements basic scheduling with a ready queue
SysTick triggers PendSV for context switch every 1ms

Signed-off-by: Vedant Malkar <vedant@example.com>
```

### Areas

Use one of these prefixes in the title for the area:

- `startup` - startup code, vector table
- `linker` - linker script changes
- `build` - Makefile, toolchain
- `boot` - inital bring up
- `scheduler` - scheduling, context switch
- `kernel` - core kernel (task, delay, yield)
- `sync` - mutex, semaphore, queue, events
- `drivers` - GPIO, UART, SysTick, etc.
- `docs` - documentation
- `app` - application / demo code

## Branches

When contributing create properly named branch to be merged

- Feature branches - `<area>/<feature>`, e.g. `drivers/gpio-driver`

## Code Format

AstraRTOS uses clang-format to ensure a consistent coding style.
The CI pipeline will automatically reject pull requests that do not match the formatting rules defined in the `.clang-format` file.

On Ubuntu/Debian, install clang-format with:
```
sudo apt install clang-format
```
You can apply the formatting directly to your modified files by running:
```
clang-format -i -style=file <modified_file.c>
```
To check without modifying:
```
clang-format --dry-run --Werror -style=file <modified_file.c>
```

## Build and Test

Before pushing, make sure your code compiles:

```bash
make clean && make
```
