/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

package org.contikios.mrm;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.PluginType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.SupportedArguments;
import org.contikios.mrm.ChannelModelLR.ParameterLR;

/**
 * This plugin allows a user to reconfigure current radio channel parameters.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("MRM Settings LR")
@PluginType(PluginType.SIM_PLUGIN)
@SupportedArguments(radioMediums = {MRM.class})
public class FormulaViewerLR extends org.contikios.cooja.VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(FormulaViewerLR.class);

  private Simulation simulation;
  private MRM radioMedium;
  private ChannelModelLR channelModel;

  private static Dimension labelDimension = new Dimension(240, 20);
  private static NumberFormat doubleFormat = NumberFormat.getNumberInstance();
  private static NumberFormat integerFormat = NumberFormat.getIntegerInstance();

  private ArrayList<JFormattedTextField> allIntegerParameters = new ArrayList<JFormattedTextField>();
  private ArrayList<JFormattedTextField> allDoubleParameters = new ArrayList<JFormattedTextField>();
  private ArrayList<JCheckBox> allBooleanParameters = new ArrayList<JCheckBox>();

  private JPanel areaGeneral;
  private JPanel areaTransmitter;
  private JPanel areaReceiver;
  private JPanel areaRayTracer;
  private JPanel areaShadowing;

  /**
   * Creates a new formula viewer.
   *
   * @param simulationToVisualize Simulation which holds the MRM channel model.
   */
  public FormulaViewerLR(Simulation simulationToVisualize, Cooja gui) {
    super("MRM Settings", gui);

    simulation = simulationToVisualize;
    radioMedium = (MRM) simulation.getRadioMedium_LR();
    channelModel = radioMedium.getChannelModel_LR();

    // -- Create and add GUI components --
    JPanel allComponents = new JPanel();
    allComponents.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    allComponents.setLayout(new BoxLayout(allComponents, BoxLayout.Y_AXIS));

    JScrollPane scrollPane = new JScrollPane(allComponents);
    scrollPane.setPreferredSize(new Dimension(500,400));
    scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
    scrollPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    setContentPane(scrollPane);

    JPanel collapsableArea;

    // General parameters
    collapsableArea = createCollapsableArea("General parameters", allComponents);
    areaGeneral = collapsableArea;

    addBooleanParameter(
        ParameterLR.apply_random,
        ParameterLR.getDescription(ParameterLR.apply_random),
        collapsableArea,
        channelModel.getParameterBooleanValue(ParameterLR.apply_random)
    );

    addDoubleParameter(
        ParameterLR.snr_threshold,
        ParameterLR.getDescription(ParameterLR.snr_threshold),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.snr_threshold)
    );

    addDoubleParameter(
        ParameterLR.bg_noise_mean,
        ParameterLR.getDescription(ParameterLR.bg_noise_mean),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.bg_noise_mean)
    );

    addDoubleParameter(
        ParameterLR.bg_noise_var,
        ParameterLR.getDescription(ParameterLR.bg_noise_var),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.bg_noise_var)
    );

    addDoubleParameter(
        ParameterLR.system_gain_mean,
        ParameterLR.getDescription(ParameterLR.system_gain_mean),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.system_gain_mean)
    );

    addDoubleParameter(
        ParameterLR.system_gain_var,
        ParameterLR.getDescription(ParameterLR.system_gain_var),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.system_gain_var)
    );

    addDoubleParameter(
        ParameterLR.frequencyLR,
        ParameterLR.getDescription(ParameterLR.frequencyLR),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.frequencyLR)
    );

    addBooleanParameter(
        ParameterLR.captureEffect,
        ParameterLR.getDescription(ParameterLR.captureEffect),
        collapsableArea,
        channelModel.getParameterBooleanValue(ParameterLR.captureEffect)
    );

    addDoubleParameter(
        ParameterLR.captureEffectPreambleDuration,
        ParameterLR.getDescription(ParameterLR.captureEffectPreambleDuration),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.captureEffectPreambleDuration)
    );

    addDoubleParameter(
        ParameterLR.captureEffectSignalTreshold,
        ParameterLR.getDescription(ParameterLR.captureEffectSignalTreshold),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.captureEffectSignalTreshold)
    );

    // Transmitter parameters
    collapsableArea = createCollapsableArea("Transmitter parameters", allComponents);
    areaTransmitter = collapsableArea;

    addDoubleParameter(
        ParameterLR.tx_power,
        ParameterLR.getDescription(ParameterLR.tx_power),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.tx_power)
    );

    addBooleanParameter(
        ParameterLR.tx_with_gain,
        ParameterLR.getDescription(ParameterLR.tx_with_gain),
        collapsableArea,
        channelModel.getParameterBooleanValue(ParameterLR.tx_with_gain)
    );

    // Receiver parameters
    collapsableArea = createCollapsableArea("Receiver parameters", allComponents);
    areaReceiver = collapsableArea;

    addDoubleParameter(
        ParameterLR.rx_sensitivityLR,
        ParameterLR.getDescription(ParameterLR.rx_sensitivityLR),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.rx_sensitivityLR)
    );

    addBooleanParameter(
        ParameterLR.rx_with_gain,
        ParameterLR.getDescription(ParameterLR.rx_with_gain),
        collapsableArea,
        channelModel.getParameterBooleanValue(ParameterLR.rx_with_gain)
    );

    // Ray Tracer parameters
    collapsableArea = createCollapsableArea("Ray Tracer parameters", allComponents);
    areaRayTracer = collapsableArea;

    addBooleanParameter(
        ParameterLR.rt_disallow_direct_path,
        ParameterLR.getDescription(ParameterLR.rt_disallow_direct_path),
        collapsableArea,
        channelModel.getParameterBooleanValue(ParameterLR.rt_disallow_direct_path)
    );

    addBooleanParameter(
        ParameterLR.rt_ignore_non_direct,
        ParameterLR.getDescription(ParameterLR.rt_ignore_non_direct),
        collapsableArea,
        channelModel.getParameterBooleanValue(ParameterLR.rt_ignore_non_direct)
    );

    addBooleanParameter(
        ParameterLR.rt_fspl_on_total_length,
        ParameterLR.getDescription(ParameterLR.rt_fspl_on_total_length),
        collapsableArea,
        channelModel.getParameterBooleanValue(ParameterLR.rt_fspl_on_total_length)
    );

    addIntegerParameter(
        ParameterLR.rt_max_rays,
        ParameterLR.getDescription(ParameterLR.rt_max_rays),
        collapsableArea,
        channelModel.getParameterIntegerValue(ParameterLR.rt_max_rays)
    );

    addIntegerParameter(
        ParameterLR.rt_max_refractions,
        ParameterLR.getDescription(ParameterLR.rt_max_refractions),
        collapsableArea,
        channelModel.getParameterIntegerValue(ParameterLR.rt_max_refractions)
    );

    addIntegerParameter(
        ParameterLR.rt_max_reflections,
        ParameterLR.getDescription(ParameterLR.rt_max_reflections),
        collapsableArea,
        channelModel.getParameterIntegerValue(ParameterLR.rt_max_reflections)
    );

    addIntegerParameter(
        ParameterLR.rt_max_diffractions,
        ParameterLR.getDescription(ParameterLR.rt_max_diffractions),
        collapsableArea,
        channelModel.getParameterIntegerValue(ParameterLR.rt_max_diffractions)
    );

/*    addBooleanParameter(
        Parameters.rt_use_scattering,
        Parameter.getDescription(Parameters.rt_use_scattering),
        collapsableArea,
        currentChannelModel.getParameterBooleanValue(Parameters.rt_use_scattering)
    );
*/
    addDoubleParameter(
        ParameterLR.rt_refrac_coefficient,
        ParameterLR.getDescription(ParameterLR.rt_refrac_coefficient),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.rt_refrac_coefficient)
    );

    addDoubleParameter(
        ParameterLR.rt_reflec_coefficient,
        ParameterLR.getDescription(ParameterLR.rt_reflec_coefficient),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.rt_reflec_coefficient)
    );

    addDoubleParameter(
        ParameterLR.rt_diffr_coefficient,
        ParameterLR.getDescription(ParameterLR.rt_diffr_coefficient),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.rt_diffr_coefficient)
    );

/*    addDoubleParameter(
        Parameters.rt_scatt_coefficient,
        Parameter.getDescription(Parameters.rt_scatt_coefficient),
        collapsableArea,
        currentChannelModel.getParameterDoubleValue(Parameters.rt_scatt_coefficient)
    );
*/
    // Shadowing parameters
    collapsableArea = createCollapsableArea("Shadowing parameters", allComponents);
    areaShadowing = collapsableArea;

    addDoubleParameter(
        ParameterLR.obstacle_attenuation,
        ParameterLR.getDescription(ParameterLR.obstacle_attenuation),
        collapsableArea,
        channelModel.getParameterDoubleValue(ParameterLR.obstacle_attenuation)
    );

    // Add channel model observer responsible to keep all GUI components synched
    channelModel.addSettingsObserver(channelModelSettingsObserver);

    // Set initial size etc.
    pack();
    setVisible(true);

    // Tries to select this plugin
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }

  }

  /**
   * Creates a new collapsable area which may be used for holding model parameters.
   * @param title Title of area
   * @param contentPane Where this area should be added
   * @return New empty collapsable area
   */
  private JPanel createCollapsableArea(String title, Container contentPane) {
    // Create panels
    JPanel holdingPanel = new JPanel() {
      private static final long serialVersionUID = -7925426641856424500L;
      public Dimension getMaximumSize() {
        return new Dimension(super.getMaximumSize().width, getPreferredSize().height);
      }
    };
    holdingPanel.setLayout(new BoxLayout(holdingPanel, BoxLayout.Y_AXIS));

    final JPanel collapsableArea = new JPanel() {
      private static final long serialVersionUID = -1261182973911973773L;
      public Dimension getMaximumSize() {
        return new Dimension(super.getMaximumSize().width, getPreferredSize().height);
      }
    };
    collapsableArea.setLayout(new BoxLayout(collapsableArea, BoxLayout.Y_AXIS));
    collapsableArea.setVisible(false);

    JPanel titlePanel = new JPanel(new BorderLayout()) {
      private static final long serialVersionUID = -9121775806029887815L;
      public Dimension getMaximumSize() {
        return new Dimension(super.getMaximumSize().width, getPreferredSize().height);
      }
    };

    titlePanel.add(BorderLayout.WEST, new JLabel(title));
    JCheckBox collapseCheckBox = new JCheckBox("show settings", false);
    collapseCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (((JCheckBox) e.getSource()).isSelected()) {
          collapsableArea.setVisible(true);
        } else {
          collapsableArea.setVisible(false);
        }
      }
    });
    collapsableArea.putClientProperty("my_checkbox", collapseCheckBox);

    titlePanel.add(BorderLayout.EAST, collapseCheckBox);

    collapsableArea.setBorder(
        BorderFactory.createLineBorder(Color.LIGHT_GRAY)
    );
    collapsableArea.setAlignmentY(Component.TOP_ALIGNMENT);

    holdingPanel.add(titlePanel);
    holdingPanel.add(collapsableArea);

    contentPane.add(holdingPanel);
    return collapsableArea;
  }

  /**
   * Creates and adds a panel with a label and a
   * text field which accepts doubles.
   *
   * @param id Identifier of new parameter
   * @param description Description of new parameter
   * @param contentPane Where to add created panel
   * @param initialValue Initial value
   * @return Text field in created panel
   */
  private JFormattedTextField addDoubleParameter(ParameterLR id, String description, Container contentPane, double initialValue) {
    JPanel panel = new JPanel();
    JLabel label;
    JFormattedTextField textField;

    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setAlignmentY(Component.TOP_ALIGNMENT);
    panel.add(Box.createHorizontalStrut(10));
    panel.add(label = new JLabel(description));
    label.setPreferredSize(labelDimension);
    panel.add(Box.createHorizontalGlue());
    panel.add(textField = new JFormattedTextField(doubleFormat));
    textField.setValue(new Double(initialValue));
    textField.setColumns(4);
    textField.putClientProperty("id", id);
    textField.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent e) {
        JFormattedTextField textField = (JFormattedTextField) e.getSource();
        ParameterLR id = (ParameterLR) textField.getClientProperty("id");
        Double val = ((Number) e.getNewValue()).doubleValue();
        channelModel.setParameterValue(id, val);
        if (!ParameterLR.getDefaultValue(id).equals(val)) {
          textField.setBackground(Color.LIGHT_GRAY);
          textField.setToolTipText("Default value: " + ParameterLR.getDefaultValue(id));
        } else {
          textField.setBackground(null);
          textField.setToolTipText(null);
        }
      }
    });
    if (!ParameterLR.getDefaultValue(id).equals(initialValue)) {
      textField.setBackground(Color.LIGHT_GRAY);
      textField.setToolTipText("Default value: " + ParameterLR.getDefaultValue(id));
    } else {
      textField.setBackground(null);
      textField.setToolTipText(null);
    }

    allDoubleParameters.add(textField);

    contentPane.add(panel);

    return textField;
  }

  /**
   * Creates and adds a panel with a label and a
   * text field which accepts integers.
   *
   * @param id Identifier of new parameter
   * @param description Description of new parameter
   * @param contentPane Where to add created panel
   * @param initialValue Initial value
   * @return Text field in created panel
   */
  private JFormattedTextField addIntegerParameter(ParameterLR id, String description, Container contentPane, int initialValue) {
    JPanel panel = new JPanel();
    JLabel label;
    JFormattedTextField textField;

    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setAlignmentY(Component.TOP_ALIGNMENT);
    panel.add(Box.createHorizontalStrut(10));
    panel.add(label = new JLabel(description));
    label.setPreferredSize(labelDimension);
    panel.add(Box.createHorizontalGlue());
    panel.add(textField = new JFormattedTextField(integerFormat));
    textField.setValue(new Double(initialValue));
    textField.setColumns(4);
    textField.putClientProperty("id", id);
    textField.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent e) {
        JFormattedTextField textField = (JFormattedTextField) e.getSource();
        ParameterLR id = (ParameterLR) textField.getClientProperty("id");
        Integer val = ((Number) e.getNewValue()).intValue();
        channelModel.setParameterValue(id, val);
        if (!ParameterLR.getDefaultValue(id).equals(val)) {
          textField.setBackground(Color.LIGHT_GRAY);
          textField.setToolTipText("Default value: " + ParameterLR.getDefaultValue(id));
        } else {
          textField.setBackground(null);
          textField.setToolTipText(null);
        }
      }
    });
    if (!ParameterLR.getDefaultValue(id).equals(initialValue)) {
      textField.setBackground(Color.LIGHT_GRAY);
      textField.setToolTipText("Default value: " + ParameterLR.getDefaultValue(id));
    } else {
      textField.setBackground(null);
      textField.setToolTipText(null);
    }

    allIntegerParameters.add(textField);

    contentPane.add(panel);

    return textField;
  }

  /**
   * Creates and adds a panel with a label and a
   * boolean checkbox.
   *
   * @param id Identifier of new parameter
   * @param description Description of new parameter
   * @param contentPane Where to add created panel
   * @param initialValue Initial value
   * @return Checkbox in created panel
   */
  private JCheckBox addBooleanParameter(ParameterLR id, String description, Container contentPane, boolean initialValue) {
    JPanel panel = new JPanel();
    JLabel label;
    JCheckBox checkBox;

    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    panel.setAlignmentY(Component.TOP_ALIGNMENT);
    panel.add(Box.createHorizontalStrut(10));
    panel.add(label = new JLabel(description));
    label.setPreferredSize(labelDimension);
    panel.add(Box.createHorizontalGlue());
    panel.add(checkBox = new JCheckBox());
    checkBox.setSelected(initialValue);
    checkBox.putClientProperty("id", id);
    checkBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        JCheckBox checkBox = (JCheckBox) e.getSource();
        ParameterLR id = (ParameterLR) checkBox.getClientProperty("id");
        Object val = new Boolean(checkBox.isSelected());
        channelModel.setParameterValue(id, val);
        if (!ParameterLR.getDefaultValue(id).equals(val)) {
          checkBox.setText("<");
        } else {
          checkBox.setText("");
        }
      }
    });
    if (!ParameterLR.getDefaultValue(id).equals(initialValue)) {
      checkBox.setText("<");
    } else {
      checkBox.setText("");
    }

    allBooleanParameters.add(checkBox);

    contentPane.add(panel);

    return checkBox;
  }

  /**
   * Listens to settings changes in the channel model.
   * If it changes, all GUI parameters are updated accordingly.
   */
  private Observer channelModelSettingsObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      // Update all integers
      for (int i=0; i < allIntegerParameters.size(); i++) {
        JFormattedTextField textField = allIntegerParameters.get(i);
        ParameterLR id = (ParameterLR) textField.getClientProperty("id");
        textField.setValue(channelModel.getParameterValue(id));
      }

      // Update all doubles
      for (int i=0; i < allDoubleParameters.size(); i++) {
        JFormattedTextField textField = allDoubleParameters.get(i);
        ParameterLR id = (ParameterLR) textField.getClientProperty("id");
        textField.setValue(channelModel.getParameterValue(id));
      }

      // Update all booleans
      for (int i=0; i < allBooleanParameters.size(); i++) {
        JCheckBox checkBox = allBooleanParameters.get(i);
        ParameterLR id = (ParameterLR) checkBox.getClientProperty("id");
        checkBox.setSelected(channelModel.getParameterBooleanValue(id));
      }

      repaint();
    }
  };

  public void closePlugin() {
    channelModel.deleteSettingsObserver(channelModelSettingsObserver);
  }

  /**
   * Returns XML elements representing the current configuration.
   *
   * @see #setConfigXML(Collection)
   * @return XML element collection
   */
  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    element = new Element("show_general");
    element.setText(Boolean.toString(areaGeneral.isVisible()));
    config.add(element);
    element = new Element("show_transmitter");
    element.setText(Boolean.toString(areaTransmitter.isVisible()));
    config.add(element);
    element = new Element("show_receiver");
    element.setText(Boolean.toString(areaReceiver.isVisible()));
    config.add(element);
    element = new Element("show_raytracer");
    element.setText(Boolean.toString(areaRayTracer.isVisible()));
    config.add(element);
    element = new Element("show_shadowing");
    element.setText(Boolean.toString(areaShadowing.isVisible()));
    config.add(element);
    return config;
  }

  /**
   * Sets the configuration depending on the given XML elements.
   *
   * @see #getConfigXML()
   * @param configXML
   *          Config XML elements
   * @return True if config was set successfully, false otherwise
   */
  public boolean setConfigXML(Collection<Element> configXML) {
    for (Element element : configXML) {
      if (element.getName().equals("show_general")) {
        JCheckBox checkBox = (JCheckBox) areaGeneral.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      } else if (element.getName().equals("show_transmitter")) {
        JCheckBox checkBox = (JCheckBox) areaTransmitter.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      } else if (element.getName().equals("show_receiver")) {
        JCheckBox checkBox = (JCheckBox) areaReceiver.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      } else if (element.getName().equals("show_raytracer")) {
        JCheckBox checkBox = (JCheckBox) areaRayTracer.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      } else if (element.getName().equals("show_shadowing")) {
        JCheckBox checkBox = (JCheckBox) areaShadowing.getClientProperty("my_checkbox");
        checkBox.setSelected(Boolean.parseBoolean(element.getText()));
        checkBox.getActionListeners()[0].actionPerformed(new ActionEvent(checkBox,
            ActionEvent.ACTION_PERFORMED, ""));
      }
    }
    return true;
  }

}
