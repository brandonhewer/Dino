import { Component } from 'react';
import { ActionMenu, IActionMenuProps } from '~/action_menu';
import { Browser } from '~/browser';
import { ComponentMenu, IComponentMenuProps } from '~/component_menu';
import { WorksheetComponent } from './worksheet-component';

interface IWorksheetProps {
  readonly components: JSX.Element[];
}

function Worksheet(props: IWorksheetProps) {
  return (
    <div id="worksheet">
      {this.props.components}
    </div>
  );
}

interface IWorkspaceProps {
  componentMenuProps: IComponentMenuProps;
  actionMenuProps: IActionMenuProps;
}

export class Workspace extends Component<IWorkspaceProps, {}> {
  private browser: JSX.Element;

  constructor(props: IWorkspaceProps) {
    super(props);
  }

  public render() {
    return (
      <div id="workspace">
        {this.browser}
      </div>
    );
  }
}
