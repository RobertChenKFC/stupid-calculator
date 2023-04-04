let fileReader;
let files;
let fileIdx;
let start = false;
let bufBegin;
let bufEnd;
let speed;
let execute;
let reset;
let getVmCode;
let getAssembly;
let getMachineCode;
let density;
let consoleLogText;
let vmCodeStrings = {};
let assemblyString;
let machineCode = [];
let progName;
const ROM_SIZE = 32768;
// TODO: change this later
const RAM_SIZE = 24577;
const WORD_SIZE = 2;
const HEAP = {
  2: "HEAP16",
  4: "HEAP32"
};
const INT_ARRAY = {
  2: Int16Array,
  4: Int32Array
};
const SEGMENT_WIDTH = 5;
const SEGMENT_HEIGHT = 18;
const MARGIN = 8;
const NUM_DIGITS = 13;
const DIGIT_WIDTH = (MARGIN + SEGMENT_WIDTH) * 2 + SEGMENT_HEIGHT;
const DIGIT_HEIGHT = (MARGIN + SEGMENT_HEIGHT) * 2 + SEGMENT_WIDTH * 3;
const DISPLAY_ADDRESS = 16384;
const KEY_ADD = 10;
const KEY_SUB = 11;
const KEY_MUL = 12;
const KEY_DIV = 13;
const KEY_SQRT = 14;
const KEY_EQUAL = 15;
const KEY_CLEAR = 16;
const KEY_POINT = 17;
const KEY_NEG = 18;
const KEY_BLANK = 19;
const BACKGROUND_COLOR = 230;

function saveFile(contents, filename, type = "text/plain") {
  const link = document.createElement("a");
  const file = new Blob([contents], { type: type });
  link.href = URL.createObjectURL(file);
  link.download = filename;
  link.click();
  URL.revokeObjectURL(link.href);
}

function saveBinaryFile(contents, filename) {
  saveFile(contents, filename, "octet/stream");
}

class SharedString {
  constructor(input) {
    const uint8Arr = new Uint8Array(input);
    this.numBytes = uint8Arr.length * uint8Arr.BYTES_PER_ELEMENT;
    this.dataPtr = Module._malloc(this.numBytes);
    this.dataOnHeap = new Uint8Array(
        Module.HEAPU8.buffer, this.dataPtr, this.numBytes);
    this.dataOnHeap.set(uint8Arr);
  }

  getPtr() {
    return this.dataOnHeap.byteOffset;
  }

  getLength() {
    return this.numBytes;
  }

  getString() {
    return new TextDecoder().decode(this.dataOnHeap);
  }

  getArrayBuffer() {
    const dest = new ArrayBuffer(this.getLength());
    new Uint8Array(dest).set(this.dataOnHeap);
    return dest;
  }

  free() {
    Module._free(this.dataPtr);
  }
}

function readFile() {
  if (fileIdx == -1) {
    return;
  } else if (fileIdx < files.length) {
    fileReader.readAsArrayBuffer(files[fileIdx]);
  } else if (fileIdx == files.length) {
    const assemblyLength = Module.ccall(
        "GetAssemblyLength", "number", [], []);
    const assembly = new SharedString(assemblyLength);
    Module.ccall("GetAssembly", null, ["number"], [assembly.getPtr()]);
    assemblyString = assembly.getString();
    assembly.free();

    Module.ccall("Assemble", null, [], []);

    const curMachineCode = new SharedString(ROM_SIZE);
    for (let i = 0; i < WORD_SIZE; ++i) {
      Module.ccall(
          "GetMachineCode", null, ["number", "number"],
          [curMachineCode.getPtr(), i]);
      machineCode.push(curMachineCode.getArrayBuffer());
    }
    curMachineCode.free();

    if (Module.ccall('InitializeExecution')) {
      execute.removeAttribute('disabled');
      reset.removeAttribute('disabled');
      getVmCode.removeAttribute("disabled");
      getAssembly.removeAttribute("disabled");
      getMachineCode.removeAttribute("disabled");
    }
  }
}

function setup() {
  window.addEventListener("keydown", function(e) {
    // space and arrow keys
    if([32, 37, 38, 39, 40].indexOf(e.keyCode) > -1) {
      e.preventDefault();
    }
  }, false);

  fileReader = new FileReader();
  fileReader.onload = (event) => {
    if (event.target.readyState != 2) return;
    if (event.target.error) {
      alert('Error while reading file! Please load again...');
      return;
    }

    const filename = files[fileIdx].name
    const fileContent = new SharedString(event.target.result);
    const vmCodeLength = Module.ccall(
        'CompileFile', "number", ['string', 'number', 'number'],
        [filename, fileContent.getPtr(), fileContent.getLength()]);
    if (vmCodeLength < 0) {
      fileIdx = -1;
    } else {
      const vmCodeString = new SharedString(vmCodeLength);
      Module.ccall("GetVmCodeString", null, ["string", "number"],
                   [filename, vmCodeString.getPtr()]);
      vmCodeStrings[filename] = vmCodeString.getString();
      vmCodeString.free();

      Module.ccall("TranslateFile", null, ["string"],
                   [filename]);

      ++fileIdx;
    }
    fileContent.free();

    readFile();
  };

  setTimeout(() => {
    Module.ccall("Init", null, [], []);

    const memory = new INT_ARRAY[WORD_SIZE](RAM_SIZE);
    const buf = Module._malloc(memory.length * memory.BYTES_PER_ELEMENT);
    bufBegin = buf / memory.BYTES_PER_ELEMENT;
    bufEnd = bufBegin + memory.length;
    Module[HEAP[WORD_SIZE]].set(memory, bufBegin);
    Module.ccall('SetMemoryPtr', null, ['number'], [buf]);

    const fileInput = document.getElementById('fileInput');
    fileInput.onclick = () => {
      fileInput.value = '';
      Module.ccall('Clear');
      start = false;
      background(BACKGROUND_COLOR);
      execute.attribute('disabled', '');
      reset.attribute('disabled', '');
      getVmCode.attribute("disabled", "");
      getAssembly.attribute("disabled", "");
      getMachineCode.attribute("disabled", "");
      consoleLogText.innerHTML = '';
      vmCodeStrings = {};
      machineCode = [];
    };
    fileInput.oninput = (event) => {
      files = event.target.files;
      fileIdx = 0;
      readFile();
    };

    execute = select('#execute');
    reset = select('#reset');

    execute.mousePressed(() => {
      execute.attribute('disabled', '');
      console.log('Starting execution...');
      start = true;
    });

    reset.mousePressed(() => {
      Module.ccall('Reset');
      background(BACKGROUND_COLOR);
      start = false;
      execute.removeAttribute('disabled');
      consoleLogText.innerHTML = '';
    });

    progName = select("#progName");
    getVmCode = select("#vmCode");
    getAssembly = select("#assembly");
    getMachineCode = select("#machineCode");
    getVmCode.mousePressed(() => {
      const zip = JSZip();
      for (let filename in vmCodeStrings) {
        zip.file(
            filename.replace(".jack", ".vm"), vmCodeStrings[filename]);
      }
      zip.generateAsync({type: "blob"}).then(zipFile => {
        saveBinaryFile(zipFile, progName.value() + ".vm.zip");
      });
    });
    getAssembly.mousePressed(() => {
      saveFile(assemblyString, progName.value() + ".asm");
    });
    getMachineCode.mousePressed(() => {
      const zip = JSZip();
      for (let i = 0; i < machineCode.length; ++i)
        zip.file(`${i}.hack`, machineCode[i]);
      zip.generateAsync({type: "blob"}).then(zipFile => {
        saveBinaryFile(zipFile, progName.value() + ".hack.zip");
      });
    });
    const speedSlider = select('#speed');
    speed = speedSlider.value();
    speedSlider.changed(() => {
      speed = speedSlider.value();
    });
  }, 1000);

  createCanvas(DIGIT_WIDTH * NUM_DIGITS, DIGIT_HEIGHT).parent('canvas');
  background(BACKGROUND_COLOR);
  density = pixelDensity();

  consoleLogText = document.getElementById('console-log-text');
}

function draw() {
  if (start) nextFrame();
}

function nextFrame() {
  if (Module.ccall('Execute', 'number', ['number'], [speed])) {
    console.log('Program halted.');
    execute.attribute('disabled', '');
    start = false;
  }

  const memory = Module[HEAP[WORD_SIZE]].subarray(bufBegin, bufEnd);

  const rects = [
      {vert: false, x: MARGIN + SEGMENT_WIDTH, y: MARGIN},
      {vert: true,  x: MARGIN + SEGMENT_WIDTH + SEGMENT_HEIGHT,
                    y: MARGIN + SEGMENT_WIDTH},
      {vert: true,  x: MARGIN + SEGMENT_WIDTH + SEGMENT_HEIGHT,
                    y: MARGIN + SEGMENT_WIDTH * 2 + SEGMENT_HEIGHT},
      {vert: false, x: MARGIN + SEGMENT_WIDTH,
                    y: MARGIN + (SEGMENT_WIDTH + SEGMENT_HEIGHT) * 2},
      {vert: true,  x: MARGIN, y: MARGIN + SEGMENT_WIDTH * 2 + SEGMENT_HEIGHT},
      {vert: true,  x: MARGIN, y: MARGIN + SEGMENT_WIDTH},
      {vert: false, x: MARGIN + SEGMENT_WIDTH,
                    y: MARGIN + SEGMENT_WIDTH + SEGMENT_HEIGHT}
  ];
  for (let i = 0; i < NUM_DIGITS; ++i) {
    const cur = memory[DISPLAY_ADDRESS + NUM_DIGITS - i - 1];
    let j = 0;
    for (const r of rects) {
      const x = DIGIT_WIDTH * i + r.x;
      const y = r.y;
      let w, h;
      if (r.vert)
        w = SEGMENT_WIDTH, h = SEGMENT_HEIGHT;
      else
        w = SEGMENT_HEIGHT, h = SEGMENT_WIDTH;

      noStroke();
      if ((cur >> j) & 1)
        fill(255, 0, 0);
      else
        fill(BACKGROUND_COLOR);
      rect(x, y, w, h);

      ++j;
    }
    const x = DIGIT_WIDTH * i + MARGIN + SEGMENT_WIDTH * 2 + SEGMENT_HEIGHT;
    const y = MARGIN + SEGMENT_WIDTH * 3 + SEGMENT_HEIGHT * 2;
    const w = SEGMENT_WIDTH, h = SEGMENT_WIDTH;
    noStroke();
    if ((cur >> 7) & 1)
      fill(255, 0, 0);
    else
      fill(BACKGROUND_COLOR);
    rect(x, y, w, h);
  }

  let pressedKey = KEY_BLANK;
  if (keyIsPressed) {
    switch (key) {
      case "0":
      case "1":
      case "2":
      case "3":
      case "4":
      case "5":
      case "6":
      case "7":
      case "8":
      case "9":
        pressedKey = parseInt(key);
        break;
      case "+":
        pressedKey = KEY_ADD;
        break;
      case "-":
        pressedKey = KEY_SUB;
        break;
      case "*":
        pressedKey = KEY_MUL;
        break;
      case "/":
        pressedKey = KEY_DIV;
        break;
      case "S":
      case "s":
        pressedKey = KEY_SQRT;
        break;
      case "=":
        pressedKey = KEY_EQUAL;
        break;
      case "C":
      case "c":
        pressedKey = KEY_CLEAR;
        break;
      case ".":
        pressedKey = KEY_POINT;
        break;
      case "N":
      case "n":
        pressedKey = KEY_NEG;
        break;
      default:
        pressedKey = KEY_BLANK;
        break;
    }
    switch (keyCode) {
      case ENTER:
        pressedKey = KEY_EQUAL;
    }
  }
  Module.ccall('SetKey', null, ['number'], [pressedKey]);
}

