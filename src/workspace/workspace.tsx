import { Component, MouseEvent } from 'react';
import { GridLayout } from 'react-grid-layout';
import { Browser, IBrowserSetup } from '~/browser';

import { 
  ActionMenu, 
  IActionMenuSetup 
} from '~/action_menu';

import { 
  ComponentMenu, 
  IComponentMenuSetup 
} from '~/component_menu';

import { WorksheetComponent } from './worksheet-component';

function removeElementAt<T>(list: T[], index: number): T[] {
  return list.slice(0, index).concat(list.slice(index + 1, list.length - 1));
}

interface IPositionedComponent {
  readonly element: JSX.Element;
  readonly position: [number, number];
};

interface IWorksheetProps {
  readonly components: IPositionedComponent[];
  readonly selectedComponent?: number;
  readonly onClick: (i: number) => any;
}

function replaceWorksheetComponent(
  components: IPositionedComponent[],
  index: number,
  element: JSX.Element
  ) {
  let newComponents = components.slice();
  newComponents[index] = {
    element: element, 
    position: components[index].position
  };
  return newComponents;
}

function createWorksheetComponents(props: IWorksheetProps): JSX.Element[] {
  return props.components.map((component, index, _) => {
    return <WorksheetComponent
      element={component.element}
      defaultPosition={{x: component.position[0], y: component.position[1]}}
      onClick={(e: MouseEvent) => props.onClick(index)}
      isSelected={index === props.selectedComponent}
    />
  });
}

function Worksheet(props: IWorksheetProps): JSX.Element {
  return (
    <div id="worksheet">
      {createWorksheetComponents(props)}
    </div>
  );
}

enum WorkspaceActions {
  ADD_ELEMENT,
  REMOVE_ELEMENT,
  REPLACE_ELEMENT,
}

export interface AddAction {
  readonly identifier: WorkspaceActions.ADD_ELEMENT;
  readonly element: JSX.Element;
}

export interface RemoveAction {
  readonly identifier: WorkspaceActions.REMOVE_ELEMENT;
  readonly index: number;
}

export interface ReplaceAction {
  readonly identifier: WorkspaceActions.REPLACE_ELEMENT;
  readonly index: number;
  readonly element: JSX.Element;
}

type Actions = AddAction | RemoveAction | ReplaceAction | undefined;
type ActionConnector = (c: ((i: number) => any)) => any;

interface IWorkspaceProps {
  readonly browserSetup: IBrowserSetup;
  readonly actionMenuSetup: IActionMenuSetup;
  readonly componentMenuSetup: IComponentMenuSetup;
  readonly actions: ((i?: number) => Actions)[];
  readonly actionConnectors: ActionConnector[];
  readonly components: JSX.Element[];
}

interface IWorkspaceState {
  components: IPositionedComponent[];
  selectedComponent?: number;
}

export class Workspace extends Component<IWorkspaceProps, IWorkspaceState> {
  private readonly windows: JSX.Element[];

  constructor(props: IWorkspaceProps) {
    super(props);
    this.state = { components: [] };

    const onAction = (i: number) => (_: MouseEvent) => this.findAndRunAction(i);
    const onRelease = (i: number) => (e: MouseEvent) => this.addComponent(i, e);

    this.props.actionConnectors.forEach(actionConnector => {
      actionConnector(this.findAndRunAction);
    });

    this.windows = [
      <ComponentMenu 
        {...this.props.componentMenuSetup} 
        onReleaseComponent={onRelease}
      />,
      <ActionMenu 
        {...this.props.actionMenuSetup} 
        onClick={onAction} 
      />
    ];
  }

  private findAndRunAction(index: number) {
    const selected = this.state.selectedComponent || undefined;
    const action = this.props.actions[index](selected);

    if (action !== undefined) {
      this.performAction(action);
    }
  }

  private performAction(action: Actions) {
    switch (action.identifier) {
      case WorkspaceActions.ADD_ELEMENT:
        this.addNewComponent(action.element, 0, 0);
        break;
      case WorkspaceActions.REMOVE_ELEMENT: 
        this.removeComponent(action.index);
        break;
      case WorkspaceActions.REPLACE_ELEMENT:
        this.replaceComponent(action.index, action.element);
        break;
    }
  }

  private selectComponent(index: number) {
    this.setState({ selectedComponent : index });
  }

  private addComponent(index: number, e: MouseEvent) {
    this.addNewComponent(this.props.components[index], e.pageX, e.pageY);
  }

  private addNewComponent(component: JSX.Element, x: number, y: number) {
    this.setState((state, _) => ({ 
      components : state.components.concat([{
        element: component,
        position: [x, y],
      }])
    }));
  }

  private removeComponent(index: number) {
    this.setState((state, _) => ({
      components : removeElementAt(state.components, index)
    }));
  }

  private replaceComponent(index: number, element: JSX.Element) {
    this.setState((state, _) => ({
      components : replaceWorksheetComponent(state.components, index, element)
    }));
  }

  public render(): JSX.Element {
    return (
      <GridLayout id='workspace' columns={2} rows={1}>
        <Browser {...this.props.browserSetup} windows={this.windows}/>;
        <Worksheet 
          components={this.state.components} 
          onClick={this.selectComponent}
        />
      </GridLayout>
    );
  }
}
