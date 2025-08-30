# import xml.etree.ElementTree as ElementTree
# from xml.etree.ElementTree import XMLSchema

from lxml import etree

from functools import reduce

import os

def validate(xml_path, schema_path):
    with open(schema_path) as f:
        schema_doc = etree.parse(f)
        schema = etree.XMLSchema(schema_doc)

    with open(xml_path) as f:
        try:
            xml_doc = etree.parse(f)
        except etree.XMLSyntaxError as e:
            print(e)
            return

    try:
        schema.assertValid(xml_doc)
    except etree.DocumentInvalid as e:
        print(e)
        return

    doc_root = xml_doc.getroot()

    # Every `flag` element should have 2^`bits` `value` elements if `constantValue` is not set
    flag_elements = doc_root.xpath("//flagsInterpretation/flag[not(@constantValue)]")
    for flag_el in flag_elements:
        bits = int(flag_el.attrib["bits"])
        expected_value_count = 2 ** bits
        value_elements = flag_el.xpath("value")

        if expected_value_count != len(value_elements):
            print(f'Wrong number of `value` tags under the `flag` tag at line {flag_el.sourceline}. There should be 2^{bits} ({expected_value_count}) `value` elements (based on the `bits` attribute on the `flag` tag) or zero `value` elements if the `constantValue` attribute is used. {len(value_elements)} `value` elements were found.')

    # Every `flag` element with `constantValue` set should not have any `value` elements
    flag_elements = doc_root.xpath("//flagsInterpretation/flag[@constantValue]")
    for flag_el in flag_elements:
        value_elements = flag_el.xpath("value")

        if len(value_elements) != 0:
            print(f'{len(value_elements)} `value` tags exist under the `flag` tag at line {flag_el.sourceline}. There should be zero `value` tags when the `constantValue` attribute is used.')

    # For all `enumInterpolation` elements, the child `enum` elements must have unique `id` attributes
    # No more than one `enum` element should have an `id` of "default" under each `enumInterpretation` element
    enum_interp_els = doc_root.xpath("//enumInterpretation")
    for enum_interp_el in enum_interp_els:
        enum_els = enum_interp_el.xpath("enum")
        enum_ids = [ enum_el.attrib["id"] for enum_el in enum_els ]
        if len(enum_ids) != len(set(enum_ids)):
            # Create dict of id -> list of elements
            enum_els_by_id = dict()
            for enum_el in enum_els:
                enum_id = enum_el.attrib["id"]
                if enum_id not in enum_els_by_id:
                    enum_els_by_id[enum_id] = []
                enum_els_by_id.get(enum_el.attrib["id"]).append(enum_el)
            duplicate_enum_els_by_id = { k:v for k,v in enum_els_by_id.items() if len(v) > 1 }
            for enum_id, duplicate_enum_els in duplicate_enum_els_by_id.items():
                print(f'Multiple `enum` tags with the same id of {enum_id} at lines {[enum_el.sourceline for enum_el in duplicate_enum_els]}. Each `enum` tag must have a unique `id` attribute within an `enumInterpretation`.')

    # No more than one `rule` element should have string content of "default" under each `conditionalInterpretation` element
    cond_interp_els = doc_root.xpath("//conditionalInterpretation")
    for cond_interp_el in cond_interp_els:
        default_rule_els = cond_interp_el.xpath("condition/rule[text()[normalize-space(.)='default']]")
        if len(default_rule_els) > 1:
            print(f'Multiple `rule` tags are set to "default" under the same `conditionalInterpretation` tag at lines {[rule_el.sourceline for rule_el in default_rule_els]}. Only one `rule` may be set to default.')

    # For all `sectionRef` elements, a `section` tag must exist under the `hff` tag, whose `name` attribute is the same as the `ref` attribute value
    # For all `sectionRef` elements, the `sizeExpr` attribute must be used iff the referenced section has the `autoSize` attribute set to false
    sectionref_els = doc_root.xpath("//sectionRef")
    for sectionref_el in sectionref_els:
        ref = sectionref_el.attrib["ref"]

        # Use lxml variable to avoid xpath injection
        matching_section_els = doc_root.xpath("//hff/section[@name=$ref]", ref = ref)
        if len(matching_section_els) == 0:
            print(f'The `sectionRef` tag at line {sectionref_el.sourceline} references {ref}, but no `section` tag has that `name`.')
        else:
            if (not "autoSize" in matching_section_els[0].attrib) or matching_section_els[0].attrib["autoSize"] == "true": # Default value is true
                if "sizeExpr" in sectionref_el.attrib:
                    print(f'The `sectionRef` tag at line {sectionref_el.sourceline} should not have a `sizeExpr` set, since the referenced {ref} `section` tag has `autoSize` set to true (default).')
            else:
                if not "sizeExpr" in sectionref_el.attrib:
                    print(f'The `sectionRef` tag at line {sectionref_el.sourceline} requires a `sizeExpr` attribute set, since the referenced {ref} `section` tag has `autoSize` set to false.')

    # For all `field` elements, there must be exactly one of either `size` or `sizeExpr` attributes
    for bad_field_el in doc_root.xpath("//field[@size and @sizeExpr]"):
        print(f'The `field` tag at line {bad_field_el.sourceline} has both `size` and `sizeExpr` attributes set, when only one should be used.')

    for bad_field_el in doc_root.xpath("//field[not(@size) and not(@sizeExpr)]"):
        print(f'The `field` tag at line {bad_field_el.sourceline} has neither `size` nor `sizeExpr` attributes set, when one must be used.')

    # The `name` attribute on `section` elements must be unique
    section_els = doc_root.xpath("/hff/section")
    section_names = [section_el.attrib["name"] for section_el in section_els ]
    if len(section_names) != len(set(section_names)):
        # Create dict of name -> list of elements
        section_els_by_name = dict()
        for section_el in section_els:
            section_name = section_el.attrib["name"]
            if section_name not in section_els_by_name:
                section_els_by_name[section_name] = []
            section_els_by_name.get(section_el.attrib["name"]).append(section_el)

        duplicate_section_els_by_name = { k:v for k,v in section_els_by_name.items() if len(v) > 1 }
        for section_name, duplicate_section_els in duplicate_section_els_by_name.items():
            print(f'Multiple `section` tags with the same name of {section_name} at lines {[section_el.sourceline for section_el in duplicate_section_els]}. Each `section` tag must have a unique `name` attribute.')

    # `section` elements that are not `root` and not `virtual` with a `collection` child element must have the `autoSize` attribute set to false
    section_els = doc_root.xpath("/hff/section[not(@root='true') and not(@virtual='true') and not(@autoSize='false')]")
    for section_el in section_els:
        collection_els = section_el.xpath("collection")
        if len(collection_els) > 0:
            print(f'The `section` tag at line {section_el.sourceline} must have the `root` attribute set to true, the `virtual` attribute set to true, or the `autoSize` attribute set to false.')

    # Only one `section` tag may have the `root` attribute set to true
    section_root_els = doc_root.xpath("/hff/section[@root='true']")
    if len(section_root_els) > 1:
        print(f'Multiple `section` tags are set to "root" at lines {[section_el.sourceline for section_el in section_root_els]}. Only one `section` may be set to root.')

if __name__ == "__main__":
    dir_path = os.path.dirname(os.path.realpath(__file__))
    schema_path = os.path.join(dir_path, 'hff.xsd')

    xml_path = os.sys.argv[1]
    validate(xml_path, schema_path)
