# rsc - Run Script Conveniently

## Description

`rsc` is a C command-line utility to run scripts from the `scripts` folder inside cwd. It displays available scripts as a numbered list.

## Prerequisites

- Make sure the scripts within the "scripts" directory have execute permissions.

## Installation

```bash
./scripts/install.sh
```

## Usage

`rsc`

* It will list the available scripts.
* Enter the number corresponding to the script you want to execute.
* The script will run in your current terminal session.

## Example

Let's say you have the following scripts in your "scripts" directory:

- `build.sh`
- `debug.sh`
- `deploy.sh`

Running `./rsc` will produce output similar to this:

```
Available scripts:
1) build.sh
2) debug.sh
3) deploy.sh
Enter script number: 
```

Entering `2` will then execute `debug.sh` script.

## License

This project is released under the MIT License.