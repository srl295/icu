// © 2020 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html

package com.ibm.icu.impl.units;

import com.ibm.icu.util.MeasureUnit;

/**
 * A class representing a single unit (optional SI or binary prefix, and dimensionality).
 */
public class SingleUnitImpl {
    /**
     * Simple unit index, unique for every simple unit, -1 for the dimensionless
     * unit. This is an index into a string list in unit.txt {ConversionUnits}.
     * <p>
     * The default value is -1, meaning the dimensionless unit:
     * isDimensionless() will return true, until index is changed.
     */
    private int index = -1;
    /**
     * SimpleUnit is the simplest form of a Unit. For example, for "square-millimeter", the simple unit would be "meter"Ò
     * <p>
     * The default value is "", meaning the dimensionless unit:
     * isDimensionless() will return true, until index is changed.
     */
    private String simpleUnitID = "";
    /**
     * Determine the power of the `SingleUnit`. For example, for "square-meter", the dimensionality will be `2`.
     * <p>
     * NOTE:
     * Default dimensionality is 1.
     */
    private int dimensionality = 1;
    /**
     * SI or binary prefix.
     */
    private MeasureUnit.MeasurePrefix unitPrefix = MeasureUnit.MeasurePrefix.ONE;

    public SingleUnitImpl copy() {
        SingleUnitImpl result = new SingleUnitImpl();
        result.index = this.index;
        result.dimensionality = this.dimensionality;
        result.simpleUnitID = this.simpleUnitID;
        result.unitPrefix = this.unitPrefix;

        return result;
    }

    public MeasureUnit build() {
        MeasureUnitImpl measureUnit = new MeasureUnitImpl(this);
        return measureUnit.build();
    }

    /**
     * Generates a neutral identifier string for a single unit which means we do not include the dimension signal.
     */
    public String getNeutralIdentifier() {
        StringBuilder result = new StringBuilder();
        int absPower = Math.abs(this.getDimensionality());

        assert absPower > 0 : "this function does not support the dimensionless single units";

        if (absPower == 1) {
            // no-op
        } else if (absPower == 2) {
            result.append("square-");
        } else if (absPower == 3) {
            result.append("cubic-");
        } else if (absPower <= 15) {
            result.append("pow");
            result.append(absPower);
            result.append('-');
        } else {
            throw new IllegalArgumentException("Unit Identifier Syntax Error");
        }

        result.append(this.getPrefix().getIdentifier());
        result.append(this.getSimpleUnitID());

        return result.toString();
    }

    /**
     * Compare this SingleUnitImpl to another SingleUnitImpl for the sake of
     * sorting and coalescing.
     * <p>
     * Takes the sign of dimensionality into account, but not the absolute
     * value: per-meter is not considered the same as meter, but meter is
     * considered the same as square-meter.
     * <p>
     * The dimensionless unit generally does not get compared, but if it did, it
     * would sort before other units by virtue of index being < 0 and
     * dimensionality not being negative.
     */
    int compareTo(SingleUnitImpl other) {
        if (dimensionality < 0 && other.dimensionality > 0) {
            // Positive dimensions first
            return 1;
        }
        if (dimensionality > 0 && other.dimensionality < 0) {
            return -1;
        }
        if (index < other.index) {
            return -1;
        }
        if (index > other.index) {
            return 1;
        }
        // TODO(icu-units#70): revisit when fixing normalization. For now we're
        // sorting binary prefixes before SI prefixes, for consistency with ICU4C.
        if (this.getPrefix().getBase() < other.getPrefix().getBase()) {
            return 1;
        }
        if (this.getPrefix().getBase() > other.getPrefix().getBase()) {
            return -1;
        }
        if (this.getPrefix().getPower() < other.getPrefix().getPower()) {
            return -1;
        }
        if (this.getPrefix().getPower() > other.getPrefix().getPower()) {
            return 1;
        }
        return 0;
    }

    /**
     * Checks whether this SingleUnitImpl is compatible with another for the purpose of coalescing.
     * <p>
     * Units with the same base unit and SI or binary prefix should match, except that they must also
     * have the same dimensionality sign, such that we don't merge numerator and denominator.
     */
    boolean isCompatibleWith(SingleUnitImpl other) {
        return (compareTo(other) == 0);
    }

    public String getSimpleUnitID() {
        return simpleUnitID;
    }

    public void setSimpleUnit(int simpleUnitIndex, String[] simpleUnits) {
        this.index = simpleUnitIndex;
        this.simpleUnitID = simpleUnits[simpleUnitIndex];
    }

    public int getDimensionality() {
        return dimensionality;
    }

    public void setDimensionality(int dimensionality) {
        this.dimensionality = dimensionality;
    }

    public MeasureUnit.MeasurePrefix getPrefix() {
        return unitPrefix;
    }

    public void setPrefix(MeasureUnit.MeasurePrefix unitPrefix) {
        this.unitPrefix = unitPrefix;
    }

    public int getIndex() {
        return index;
    }

}