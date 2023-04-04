const prompt = require("prompt-sync")({sigint: true});

function showDigits() {
  /*
  Seven segment display:
  ┌─┐   ╷ ╶─┐ ╶─┐ ╷ ╷ ┌─╴ ┌─╴ ╶─┐ ┌─┐ ┌─┐
  │ │   │ ┌─┘ ╶─┤ └─┤ └─┐ ├─┐   │ ├─┤ └─┤
  └─┘.  ╵.└─╴.╶─┘.  ╵.╶─┘.└─┘.  ╵.└─┘.╶─┘.
  */

  const rows = ["", "", ""];

  for (let i = NUM_DISPLAY_DIGITS; i >= 0; --i) {
    const segments = DISPLAY[i];

    const a = (segments & SEG_A) !== 0;
    const b = (segments & SEG_B) !== 0;
    const c = (segments & SEG_C) !== 0;
    const d = (segments & SEG_D) !== 0;
    const e = (segments & SEG_E) !== 0;
    const f = (segments & SEG_F) !== 0;
    const g = (segments & SEG_G) !== 0;

    if (a) {
      if (f) rows[0] += "┌";
      else rows[0] += "╶";
    } else {
      if (f) rows[0] += "╷";
      else rows[0] += " ";
    }
    if (a) rows[0] += "─";
    else rows[0] += " ";
    if (a) {
      if (b) rows[0] += "┐";
      else rows[0] += "╴";
    } else {
      if (b) rows[0] += "╷";
      else rows[0] += " ";
    }

    if (f) {
      if (e) {
        if (g) rows[1] += "├";
        else rows[1] += "│";
      } else {
        if (g) rows[1] += "└";
        else rows[1] += "╵";
      }
    } else {
      if (e) {
        if (g) rows[1] += "┌";
        else rows[1] += "╷";
      } else {
        if (g) rows[1] += "╶";
        else rows[1] += " ";
      }
    }
    if (g) rows[1] += "─";
    else rows[1] += " ";
    if (b) {
      if (c) {
        if (g) rows[1] += "┤";
        else rows[1] += "│";
      } else {
        if (g) rows[1] += "┘";
        else rows[1] += "╵";
      }
    } else {
      if (c) {
        if (g) rows[1] += "┐";
        else rows[1] += "╷";
      } else {
        if (g) rows[1] += "╴";
        else rows[1] += " ";
      }
    }

    if (d) {
      if (e) rows[2] += "└";
      else rows[2] += "╶";
    } else {
      if (e) rows[2] += "╵";
      else rows[2] += " ";
    }
    if (d) rows[2] += "─";
    else rows[2] += " ";
    if (d) {
      if (c) rows[2] += "┘";
      else rows[2] += "╴";
    } else {
      if (c) rows[2] += "╵";
      else rows[2] += " ";
    }

    rows[0] += " ";
    rows[1] += " ";
    if (segments & SEG_H)
      rows[2] += ".";
    else
      rows[2] += " ";
  }

  for (let row of rows)
    console.log(row);
}

function readKey() {
  showDigits();
  const command = prompt("Enter key: ");
  switch (command) {
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
      return parseInt(command);
    case "+":
      return KEY_ADD;
    case "-":
      return KEY_SUB;
    case "*":
      return KEY_MUL;
    case "/":
      return KEY_DIV;
    case "S":
    case "s":
      return KEY_SQRT;
    case "=":
      return KEY_EQUAL;
    case "C":
    case "c":
      return KEY_CLEAR;
    case ".":
      return KEY_POINT;
    case "N":
    case "n":
      return KEY_NEG;
    default:
      return KEY_BLANK;
  }
}

function alloc(numElems) {
  return Array(numElems);
}

main()
