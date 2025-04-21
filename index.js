import fe from './public/FlowerEvolver.mjs';
export default fe;

/*!
 * @license Apache 2.0
 * @cristianglezm/flower-evolver-wasm
 * Copyright 2023-2025 Cristian Gonzalez <cristian.glez.m@gmail.com>
 */

/**
 * @brief params to make flowers
 */
export class FEParams{
    /**
     * @param {number} radius - radius of the flower (default: 64)
     * @param {number} numLayers - how many layers it will have. (default: 3)
     * @param {number} P - P parameter (default: 6.0)
     * @param {number} bias - bias (default: 1.0)
     */
    constructor(radius = 64, numLayers = 3, P = 6.0, bias = 1.0){
        this.radius = radius;
        this.numLayers = numLayers;
        this.P = P;
        this.bias = bias;
    }
};
/**
 * @brief Utility function to check if inside worker.
 * @param {HTMLCanvasElement | OffscreenCanvas} canvas to extract the image
 */
export const getDataUrl = async (canvas) => {
    if(canvas instanceof OffscreenCanvas){
        let blob = await canvas.convertToBlob();
        let frs = new FileReaderSync();
        return frs.readAsDataURL(blob);
    }
    return canvas.toDataURL();
};
/**
 * @brief A simple Flower
 */
export class Flower{
    /**
     * @param {string} genome - stringified json
     * @param {string} image  - formatted in base64
     */
    constructor(genome, image){
        this.genome = genome;
        this.image = image;
    }
};
/**
 * @brief simple wrapper around fe wasm module.
 * @details
 * @code
 *  let fes = new FEService();
 *  // make a flower with default params
 *  let flowers = new Array();
 *  flowers.push(await fes.makeFlower());
 *  // radius, numLayers, P, bias
 *  fes.setParams(new FEParams(128, 2, 5.0, -1.0));
 *  // make a flower with new params.
 *  flowers.push(await fes.makeFlower());
 *  flowers.push(await fes.reproduce(flowers[0].genome, flowers[1].genome));
 *  // mutate child using default mutation rates.
 *  flowers.push(await fes.mutate(flowers[2].genome))
 * @endcode
 */
export class FEService{
    static fe = null;
    static canvas = null;
    static params = null;
    constructor(){
        this.params = new FEParams();
         if(!this.fe){
            if(typeof importScripts === 'function'){
                if(!self.canvas){
                    this.canvas = new OffscreenCanvas(this.params.radius*2, this.params.radius*3);
                    self.canvas = this.canvas;
                }else{
                    this.canvas = self.canvas;
                    this.setParams(this.params);
                }
            }else{
                this.canvas = document.getElementById("canvas");
                if(!this.canvas){
                    this.canvas = document.createElement("canvas");
                    this.canvas.setAttribute('id', 'canvas');
                    this.canvas.setAttribute('width', this.params.radius*2);
                    this.canvas.setAttribute('height', this.params.radius*3);
                    this.canvas.setAttribute('hidden', true);
                    document.body.appendChild(this.canvas);
                }
            }
         }
    }
    /**
     * @brief function to load wasm module.
     */
    async init(){
        if(this.fe){
            return;
        }
        try{
            this.fe = await fe();
        }catch(e){
            console.error("Error loading WASM module:", e);
        }
    }
    /**
     * @brief sets the parameters for the creation of flowers.
     * @param {FEParams} params 
     */
    setParams(params){
        this.params = params;
        this.canvas.width = this.params.radius * 2;
        this.canvas.height = this.params.radius * 3;
    }
    /**
     * @brief gets the currents parameters
     * @returns {FEParams}
     */
    getParams(){
        return this.params;
    }
    /**
     * @brief it makes a flower.
     * @returns {Flower} flower
     */
    async makeFlower(){
        if(!this.fe){
            throw Error("call FEService.init() before using it");
        }
        try{
            let genome = this.fe.makeFlower(this.params.radius, this.params.numLayers, this.params.P, this.params.bias);
            let image = await getDataUrl(this.canvas);
            return new Flower(genome, image);
        }catch(e){
            throw Error(this.fe.getExceptionMessage(e));
        }
    }
    /**
     * @brief it makes a flower with no stem.
     * @returns {Flower} flower, only petals
     */
    async makePetals(){
        if(!this.fe){
            throw Error("call FEService.init() before using it");
        }
        this.canvas.height = this.params.radius * 2;
        try{
            let genome = this.fe.makePetals(this.params.radius, this.params.numLayers, this.params.P, this.params.bias);
            let image = await getDataUrl(this.canvas);
            return new Flower(genome, image);
        }catch(e){
            throw Error(this.fe.getExceptionMessage(e));
        }
    }
    /**
     * @brief it makes the petal for the specified layer. (no stem)
     * @param {number} layer
     * @returns {Flower} flower, only petal in layer
     */
    async makePetalLayer(layer){
        if(!this.fe){
            throw Error("call FEService.init() before using it");
        }
        this.canvas.height = this.params.radius * 2;
        try{
            let genome = this.fe.makePetalLayer(this.params.radius, this.params.numLayers, this.params.P, this.params.bias, layer);
            let image = await getDataUrl(this.canvas);
            return new Flower(genome, image);
        }catch(e){
            throw Error(this.fe.getExceptionMessage(e));
        }
    }
    /**
     * @brief it makes a simple stem. (no petals)
     * @returns {Flower} flower, only stem
     */
    async makeStem(){
        if(!this.fe){
            throw Error("call FEService.init() before using it");
        }
        try{
            let genome = this.fe.makeStem(this.params.radius, 1, 1.0, 6.0, 1.0);
            let image = await getDataUrl(this.canvas);
            return new Flower(genome, image);
        }catch(e){
            throw Error(this.fe.getExceptionMessage(e));
        }
    }
    /**
     * @brief it drawn the flower from the given genome.
     * @param {string} genome - stringified json
     * @returns {Flower} flower from the given genome.
     */
    async drawFlower(genome){
        if(!this.fe){
            throw Error("call FEService.init() before using it");
        }
        try{
            this.fe.drawFlower(genome, this.params.radius, this.params.numLayers, this.params.P, this.params.bias);
            let image = await getDataUrl(this.canvas);
            return new Flower(genome, image);
        }catch(e){
            throw Error(this.fe.getExceptionMessage(e));
        }
    }
    /**
     * @brief it drawns the petals from the given genome. (no stem)
     * @param {string} genome - stringified json
     * @returns {Flower} flower petals
     */
    async drawPetals(genome){
        if(!this.fe){
            throw Error("call FEService.init() before using it");
        }
        this.canvas.height = this.params.radius * 2;
        try{
            this.fe.drawPetals(genome, this.params.radius, this.params.numLayers, this.params.P, this.params.bias);
            let image = await getDataUrl(this.canvas);
            return new Flower(genome, image);
        }catch(e){
            throw Error(this.fe.getExceptionMessage(e));
        }
    }
    /**
     * @brief it drawns the petal layer from the given genome.(no stem)
     * @param {string} genome - stringified json
     * @param {number} layer - layer to draw
     * @returns {Flower} flower - petal from specified layer
     */
    async drawPetalLayer(genome, layer){
        if(!this.fe){
            throw Error("call FEService.init() before using it");
        }
        this.canvas.height = this.params.radius * 2;
        try{
            this.fe.drawPetalLayer(genome, this.params.radius, this.params.numLayers, this.params.P, this.params.bias, layer);
            let image = await getDataUrl(this.canvas);
            return new Flower(genome, image);
        }catch(e){
            throw Error(this.fe.getExceptionMessage(e));
        }
    }
    /**
     * @bief it makes a child from two flower genomes.
     * @param {string} father - genome for father
     * @param {string} mother - genome for mother
     * @returns {Flower} flower - child flower
     */
    async reproduce(father, mother){
        if(!this.fe){
            throw Error("call FEService.init() before using it");
        }
        try{
            let genome = this.fe.reproduce(father, mother, this.params.radius, this.params.numLayers, this.params.P, this.params.bias);
            let image = await getDataUrl(this.canvas);
            return new Flower(genome, image);
        }catch(e){
            throw Error(this.fe.getExceptionMessage(e));
        }
    }
    /**
     * @brief it mutates the given flower genome.
     * @param {string} original - genome from original flower.
     * @param {number} addNodeRate 
     * @param {number} addConnRate 
     * @param {number} removeConnRate 
     * @param {number} perturbWeightsRate 
     * @param {number} enableRate 
     * @param {number} disableRate 
     * @param {number} actTypeRate 
     * @returns {Flower} flower - the new mutated flower
     */
    async mutate(original, addNodeRate = 0.2, addConnRate = 0.3, 
        removeConnRate = 0.2, perturbWeightsRate = 0.6, 
        enableRate = 0.35, disableRate = 0.3, actTypeRate = 0.4){
            if(!this.fe){
                throw Error("call FEService.init() before using it");
            }
            try{
                let genome = this.fe.mutate(original, this.params.radius, this.params.numLayers, this.params.P, this.params.bias,
                    addNodeRate, addConnRate, removeConnRate, perturbWeightsRate, 
                    enableRate, disableRate, actTypeRate);
                let image = await getDataUrl(this.canvas);
                return new Flower(genome, image);
            }catch(e){
                throw Error(this.fe.getExceptionMessage(e));
            }
    }
};