//God of War (2018/Ragnorok) Style Combo Tester in JS

const Inputs = [
    "L L L L L L",
    "H H H",
    "L H L H L H",
    "H H L L L",
    "H H L L L L",
    "H H L L H",
    "H H L H",
    "L L H L L",
    "H L L H H"
];

function clamp(number, min, max) {
    return Math.max(min, Math.min(Math.round(number), max));
  }
  
const LightLength = 6;
const HeavyLength = 3;

function ValidateInput(input_str) {
    //remove spaces
    input_str = input_str.replace(/\s/g, '');

    let comboStr = "";
    let lastInput = undefined;
    let LightIndex = 0;
    let HeavyIndex = 0;
    for(let i = 0; i < input_str.length; i++) {
        if(LightIndex >= LightLength || HeavyIndex >= HeavyLength) {
            console.log("Combo Exeeded @ Light=[" + LightIndex + "/" + LightLength + "]"
            + "Heavy=[" + HeavyIndex + "/" + HeavyLength + "]");
            break;
        }

        let combo = input_str[i];

        if(lastInput !== undefined && lastInput !== combo) {
            let base = (lastInput === "L" ? LightIndex : HeavyIndex);
            let div = (lastInput === "L" ? LightLength : HeavyLength);
            let prog = (base/div);

            if(combo === "H") {
                //do Heavy
                let cHeavy = HeavyIndex;
                HeavyIndex = Math.max(clamp(prog * HeavyLength, 1, HeavyLength), cHeavy+1);
            }
            else {
                //do Light
                let cLight = LightIndex;
                LightIndex = Math.max(clamp(prog * LightLength, 1, LightLength), cLight+1);
            }
        }
        else {
            if(combo === "L") { LightIndex++; }
            else if(combo === "H") { HeavyIndex++; }
        }

        lastInput = combo;
        comboStr += "| " + combo + (combo === "L" ? LightIndex : HeavyIndex);
    }

    comboStr += "|"
    return comboStr;
}

let DefaultPad = 20;

function Main() {
    let finalInputs = "";
    for(let i = 0; i < Inputs.length; i++) {
        finalInputs = Inputs[i].padEnd(20, ' ')
            + "     |     "
            + ValidateInput(Inputs[i]);
        console.log(finalInputs);
    }
}

Main();