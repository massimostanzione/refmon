# refmon
*A reference monitor for enhanced file protection*.
## What is it?
`refmon` is the final project for the *Advanced Operating Systems (and Systems Security)* course (9 CFU = 9 ETCS), followed at *University of Rome Tor Vergata*. 
It aims to the implementation of a **Linux Kernel Module** for a **Reference Monitor**, according to some specification that you can consult [on the official course site](https://francescoquaglia.github.io/TEACHING/AOS/AA-2023-2024/PROJECTS/project-specification-2023-2024.html).

## How to run `refmon`
### Quickstart
If you want to *just run* the project, follow this simple steps:

 1. **Clone** the repo:
 

		git clone https://github.com/massimostanzione/refmon --recurse-submodules

> [!IMPORTANT] 
> Please don't forget **`--recurse-submodules`**, as there are some [external modules](https://github.com/massimostanzione/refmon/edit/main/README.md#external-modules) that must be added in order to refmon to be fully functioning.

 2. **Launch** refmon:
 

		cd refmon
		make run
    

 3. **Play!**
That's it, you are all set! You can try to add some paths to `refmon`:

		sudo refmon --set-state REC-ON
		sudo refmon --reconf-add PATH
    
... and try to write-access your `PATH`. You should be able to get an error message, *i.e.* `EACCESS` errno.

### Keep it calm
Now that you have enjoyed the *quickstart*, feel free to explore every `refmon` functionality by typing

    refmon --help
A list of all the commands available will be displayed. Use it as your guide!

## External modules

This project uses the following external modules:

 - [*Linux syscall table discoverer*](https://github.com/FrancescoQuaglia/Linux-sys_call_table-discoverer) module, by the lecturer [Francesco Quaglia](https://github.com/FrancescoQuaglia), in order to hack some free kernel entries and use them for `refmon` purposes;
 - [*Munit*](https://github.com/nemequ/munit) test framework, by [nemequ](https://github.com/nemequ), in order to test `refmon` with a lightweight, *nuisance-free* test framework.

## Can I contribute?
Feel free to report any bug or to contribute!
