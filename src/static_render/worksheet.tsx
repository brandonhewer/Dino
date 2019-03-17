import { Component } from 'react';
import MouseTrap from 'mousetrap';
import { DropTarget } from 'react-dnd';
import { remote } from 'electron';
import { writeFile } from 'fs';
import {
  PetriCompositeComponent, 
  IPetriNet, 
  ItemTypes,
  Selection, 
  PetriTypeComponent 
} from '../petri_nets';
import svg2png from 'svg2png';

const bindings = require('bindings');
const naturality = bindings('Naturality.node');

interface ITransformation {
  readonly graph: (x: string | number) => IPetriNet;
  readonly setCospan: (x: string) => ITransformation;
  readonly cospanString: () => string;
  readonly string: () => string;
  readonly compose: (right: ITransformation) => ITransformation;
  readonly variable: (x: number) => string;
}

interface IPetriNetModel {
  transformation: ITransformation;
  graph: IPetriNet;
  cospan: string;
  variable: string;
  transform: string;
  top: number;
  left: number;
  composite: boolean;
};

interface IWorksheetProps {
  readonly componentHeight: number;
  readonly componentWidth: number;
  readonly addPetriNetHotKeys: string[];
  readonly removePetriNetHotKeys: string[];
  readonly composePetriNetHotKeys: string[];
  readonly savePetriNetHotKeys: string[];
  readonly connectDropTarget?: any;
};

interface IWorksheetState {
  primarySelected: number;
  secondarySelected: number;
  models: IPetriNetModel[];
};

const worksheetTarget = {
  drop(_: IWorksheetProps, monitor: any, component: any) {
    if (!component) {
      return;
    }
    const item = monitor.getItem();
    const delta = monitor.getDifferenceFromInitialOffset();
    const left = Math.round(item.left + delta.x);
    const top = Math.round(item.top + delta.y);
    component.moveComponent(item.index, top, left);
  }
}

function deleteAt<T>(arr: T[], index: number): T[] {
  return arr.slice(0, index).concat(arr.slice(index + 1));
}

function emptyGraph(): IPetriNet {
  return {
    nodes: [],
    transitions: [],
    edges: {
      incoming: [],
      outgoing: [],
      invisible: [],
    },
  };
}

function generateGraph(model: IPetriNetModel): IPetriNet {
  if (model.variable.length !== 0) {
    try {
      return model.transformation.graph(model.variable);
    } catch (err) {
      return emptyGraph();
    }
  }
  return emptyGraph();
}

function setCospan(models: IPetriNetModel[], index: number, cospan: string): IPetriNetModel[] {
  let newModels = models.slice(0, index);
  let modified = models[index];
  modified.cospan = cospan;
  
  try {
    modified.transformation.setCospan(cospan);
    modified.graph = generateGraph(modified);
  } catch (err) {}

  newModels.push(modified);
  return newModels.concat(models.slice(index + 1));
}

function setVariable(models: IPetriNetModel[], index: number, variable: string): IPetriNetModel[] {
  let newModels = models.slice(0, index);
  let modified = models[index];
  modified.variable = variable;
  modified.graph = generateGraph(modified);
  newModels.push(modified);
  return newModels.concat(models.slice(index + 1));
}

function setTransform(models: IPetriNetModel[], index: number, transform: string): IPetriNetModel[] {
  let newModels = models.slice(0, index);
  let modified = models[index];

  try {
    modified.transformation = naturality.createTransformation(transform);
    modified.cospan = modified.transformation.cospanString();
    modified.graph = generateGraph(modified);
  } catch (err) {}

  modified.transform = transform;
  newModels.push(modified);
  return newModels.concat(models.slice(index + 1));
}

function setPosition(models: IPetriNetModel[], index: number, top: number, left: number) {
  let newModels = models.slice(0, index);
  let modified = models[index];
  modified.top = top;
  modified.left = left;
  newModels.push(modified);
  return newModels.concat(models.slice(index + 1));
}

function emptyModel(): IPetriNetModel {
  return {
    transformation: undefined,
    graph: emptyGraph(),
    cospan: '',
    variable: '',
    transform: '',
    top: 0,
    left: 0,
    composite: false,
  };
}

const saveAsPNGOptions = {
  type: 'question',
  buttons: ['Yes, please', 'No, thanks'],
  defaultId: 2,
  title: 'Question',
  message: 'Many browsers do not support patterned SVG files, convert to PNG?',
};

function saveToFile(content: any, filePath: string) {
  writeFile(filePath, content, err => {
    if (err) {
      alert("An error occurred when creating the file: " + err.message);
    }
  });
}

class Worksheet extends Component<IWorksheetProps, IWorksheetState> {
  svgReferences: SVGSVGElement[];

  constructor(props: IWorksheetProps) {
    super(props);

    this.state = {
      primarySelected: -1,
      secondarySelected: -1,
      models: [],
    };

    this.svgReferences = [];

    MouseTrap.bind(this.props.addPetriNetHotKeys, this.addComponent.bind(this));
    MouseTrap.bind(this.props.removePetriNetHotKeys, this.removeComponent.bind(this));
    MouseTrap.bind(this.props.composePetriNetHotKeys, this.composeComponents.bind(this));
    MouseTrap.bind(this.props.savePetriNetHotKeys, this.saveSVGReference.bind(this));
  }

  private composeComponents() {
    if (this.state.primarySelected < 0 || this.state.secondarySelected < 0)
      return;
  
    const left = this.state.models[this.state.primarySelected];
    const right = this.state.models[this.state.secondarySelected];
    const composed = left.transformation.compose(right.transformation);

    this.setState({
      models: [...this.state.models, {
        left: (left.left + right.left) / 2.0,
        top: (left.top + right.top) / 2.0,
        variable: composed.variable(0),
        cospan: composed.cospanString(),
        transform: composed.string(),
        graph: composed.graph(0),
        transformation: composed,
        composite: true,
      }],
    });
  }

  private selectComponent(shift: boolean, i: number) {
    if (shift && this.state.primarySelected >= 0) {
      if (this.state.secondarySelected >= 0) {
        this.setState({
          primarySelected: this.state.secondarySelected,
          secondarySelected: i,
        });
      } else {
        this.setState({ secondarySelected: i });
      }
    } else {
      this.setState({ primarySelected: i, secondarySelected: -1 });
    }
  }

  private unselectAll() {
    this.setState({
      primarySelected: -1,
      secondarySelected: -1,
    });
  }

  private addComponent() {
    this.svgReferences.push(undefined);
    this.setState({
      models: [...this.state.models, emptyModel()],
      primarySelected: this.state.models.length,
    });
  }

  private removePrimaryComponent() {
    this.svgReferences = deleteAt(this.svgReferences, this.state.primarySelected);
    
    this.setState({
      models: deleteAt(this.state.models, this.state.primarySelected),
      primarySelected: -1,
    });
  }

  private removeSecondaryComponent() {
    this.svgReferences = deleteAt(this.svgReferences, this.state.secondarySelected);

    if (this.state.secondarySelected < this.state.primarySelected) {
      this.setState({
        models: deleteAt(this.state.models, this.state.secondarySelected),
        secondarySelected: -1,
        primarySelected: this.state.primarySelected - 1,
      });
    } else {
      this.setState({
        models: deleteAt(this.state.models, this.state.secondarySelected),
        secondarySelected: -1,
      });
    }
  }

  private removeComponent() {
    if (this.state.secondarySelected >= 0) {
      this.removeSecondaryComponent();
    } else {
      this.removePrimaryComponent();
    }
  }

  private setTransformation(index: number, transform: string) {
    this.setState({
      models: setTransform(this.state.models, index, transform),
    });
  }

  private setCospan(index: number, cospan: string) {
    this.setState({
      models: setCospan(this.state.models, index, cospan),
    });
  }

  private setVariable(index: number, variable: string) {
    this.setState({
      models: setVariable(this.state.models, index, variable),
    });
  }

  public moveComponent(index: number, top: number, left: number) {
    this.setState({
      models: setPosition(this.state.models, index, top, left),
    });
  }

  private getSelection(index: number): Selection {
    if (this.state.primarySelected === index)
      return Selection.PRIMARY;
    else if (this.state.secondarySelected === index)
      return Selection.SECONDARY;
    return Selection.NONE;
  }

  private setSVGReference(index: number, ref: SVGSVGElement) {
    this.svgReferences[index] = ref;
  }

  private saveSVGReference() {
    if (this.state.primarySelected >= 0) {
      const path = remote.dialog.showSaveDialog(null);
      remote.dialog.showMessageBox(saveAsPNGOptions, (response, _) => {
        const toSave = this.svgReferences[this.state.primarySelected].outerHTML;
        if (response == 0) {
          svg2png(toSave).then(buffer => writeFile(path, buffer, err => {
            if (err) {
              alert("An error occurred when creating the file: " + err.message);
            }
          }));
        } else {
          saveToFile(toSave, path);
        }
      });
    }
  }

  private getPetriTypeComponent(model: IPetriNetModel, index: number) {
    return (
      <PetriTypeComponent
        key={index}
        index={index}
        width={this.props.componentWidth}
        height={this.props.componentHeight}
        top={model.top}
        left={model.left}
        selected={this.getSelection(index)}
        onClicked={(shift) => this.selectComponent(shift, index)}
        liveColour={'#666'}
        deadColour={'#333'}
        placeSize={18}
        transitionSize={28}
        cospan={model.cospan}
        variable={model.variable}
        graph={model.graph}
        transform={model.transform}
        setCospan={(cospan: string) => this.setCospan(index, cospan)}
        setVariable={(variable: string) => this.setVariable(index, variable)}
        setTransform={(transform: string) => this.setTransformation(index, transform)}
        setSVGReference={(ref: SVGSVGElement) => this.setSVGReference(index, ref)}
      />
    );
  }

  private getCompositeComponent(model: IPetriNetModel, index: number) {
    return (
      <PetriCompositeComponent
        key={index}
        index={index}
        width={this.props.componentWidth}
        height={this.props.componentHeight}
        top={model.top}
        left={model.left}
        selected={this.getSelection(index)}
        onClicked={(shift) => this.selectComponent(shift, index)}
        liveColour={'#666'}
        deadColour={'#333'}
        placeSize={18}
        transitionSize={28}
        variable={model.variable}
        graph={model.graph}
        transform={model.transform}
        setVariable={(variable: string) => this.setVariable(index, variable)}
        setSVGReference={(ref: SVGSVGElement) => this.setSVGReference(index, ref)}
      />
    );
  }

  render() {
    const { connectDropTarget } = this.props;
    return connectDropTarget(
      <div 
        style={{
          width:'100vw',
          height:'100vh',
          position:'relative',
        }}
        className='worksheet'
      >
        {this.state.models.map((model, index) => {
          return model.composite ? this.getCompositeComponent(model, index) : this.getPetriTypeComponent(model, index);
        })}
      </div>
    );
  }
}

const collect = (connect, _) => ({
  connectDropTarget: connect.dropTarget(),
});

export default DropTarget([ItemTypes.PETRI_NET, ItemTypes.COMPOSITE_NET], worksheetTarget, collect)(Worksheet);
