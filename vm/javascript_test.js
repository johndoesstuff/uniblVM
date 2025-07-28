const fs = require('fs');

// Constants
const MEM_SIZE = 2 ** 20; // 1MB memory
const ENTRY_POINT = 0x0800;

// Opcodes
const OPCODES = {
HALT: 0,
      LDA: 1,
      STA: 2,
      SWP: 3,
      JMP: 4,
      JMPBZ: 5,
      ADDAB: 6,
      SUBAB: 7,
      CMPAB: 8,
      VOID: 9,
      NANDAB: 10,
      SHRA: 11,
};

// Memory and registers
const MEM = new Uint8Array(MEM_SIZE);
let ACC_A = 0n;
let ACC_B = 0n;
let PC = BigInt(ENTRY_POINT);

let DEBUG = true;
let EXPAND = true;

// Helpers
function read_u8() {
	const val = MEM[Number(PC)];
	PC++;
	return BigInt(val);
}

function read_u64() {
	let val = 0n;
	for (let i = 0; i < 8; i++) {
		val |= read_u8() << BigInt(i * 8);
	}
	return val;
}

function write_debug(op, args) {
	if (DEBUG) {
		process.stdout.write(op.padEnd(11));
		if (args) process.stdout.write(args.map(e => String(e).padEnd(6)).join("").padEnd(6*3));
		else process.stdout.write("".padEnd(6*3));

		process.stdout.write(`\t\t\tA=0x${ACC_A.toString(16).padEnd(8)}`);
		process.stdout.write(`\tB=0x${ACC_B.toString(16).padEnd(8)}`);
		process.stdout.write(`\tPC=0x${PC.toString(16).padEnd(8)}\n`);
	}
}

// Load binary
function load_binary(path) {
	const data = fs.readFileSync(path);
	MEM.set(data, ENTRY_POINT);
}

// Main execution loop
function run() {
	while (true) {
		const op = Number(read_u8());
		const instr_PC = PC;

		switch (op) {
			case OPCODES.HALT:
				return;

			case OPCODES.LDA: {
						  const offset = Number(read_u8()) * 8;
						  const addr = read_u64();
						  const width = Number(read_u8());
						  for (let i = 0; i < width; i++) {
							  const byte = MEM[Number(addr + BigInt(i))];
							  ACC_A &= ~(0xffn << BigInt(offset + i * 8));
							  ACC_A |= BigInt(byte) << BigInt(offset + i * 8);
						  }
						  write_debug("LDA", [offset.toString(16),addr,width]);
						  break;
					  }

			case OPCODES.STA: {
						  const addr = read_u64();
						  const offset = Number(read_u8()) * 8;
						  const width = Number(read_u8());
						  for (let i = 0; i < width; i++) {
							  const val = (ACC_A >> BigInt(offset + i * 8)) & 0xffn;
							  MEM[Number(addr + BigInt(i))] = Number(val);
						  }
						  write_debug("STA", [addr.toString(16),offset,width]);
						  break;
					  }

			case OPCODES.SWP:
					  [ACC_A, ACC_B] = [ACC_B, ACC_A];
					  write_debug("SWP", []);
					  break;

			case OPCODES.JMP: {
						  const addr = read_u64();
						  PC = addr;
						  write_debug("JMP", [addr.toString(16)]);
						  break;
					  }

			case OPCODES.JMPBZ: {
						    const addr = read_u64();
						    if (ACC_B === 0n) PC = addr;
						    write_debug("JMPBZ", [addr.toString(16)]);
						    break;
					    }

			case OPCODES.ADDAB:
					    ACC_A = (ACC_A + ACC_B) & 0xFFFFFFFFFFFFFFFFn;
					    write_debug("ADDAB", []);
					    break;

			case OPCODES.SUBAB:
					    ACC_A = (ACC_A >= ACC_B) ? (ACC_A - ACC_B) : 0n;
					    write_debug("SUBAB", []);
					    break;

			case OPCODES.CMPAB:
					    ACC_B = (ACC_A === ACC_B) ? 0n : 1n;
					    write_debug("CMPAB", []);
					    break;

			case OPCODES.VOID: {
						   const val = read_u64(); // do nothing
						   write_debug("VOID", [val.toString(16)]);
						   break;
					   }

			case OPCODES.NANDAB:
					   ACC_A = ~(ACC_A & ACC_B) & 0xFFFFFFFFFFFFFFFFn;
					   write_debug("NANDAB", []);
					   break;

			case OPCODES.SHRA: {
						   const offset = Number(read_u8());
						   ACC_A = ACC_A >> BigInt(offset);
						   write_debug("SHRA", [offset]);
						   break;
					   }

			default:
					   console.error(`Invalid opcode ${op} at PC=${instr_PC}`);
					   return;
		}

		if (EXPAND) {
			/*write_debug(
					`\tA=0x${ACC_A.toString(16)}\tB=0x${ACC_B.toString(16)}\tPC=0x${instr_PC.toString(16)}`
				   );*/
		}
	}
}

// CLI Entry
const args = process.argv.slice(2);
if (args.length === 0) {
	console.log("Usage: node unibl-vm.js program.bin [-d|-e]");
	process.exit(0);
}

for (const arg of args) {
	if (arg === "-d") DEBUG = true;
	else if (arg === "-e") { DEBUG = true; EXPAND = true; }
	else load_binary(arg);
}

run();

// Print stdout
let out = "";
for (let i = 0x0400; i <= 0x07FF; i++) {
	const ch = MEM[i];
	out += String.fromCharCode(ch);
}
process.stdout.write(out);

